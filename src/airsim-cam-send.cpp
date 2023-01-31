#include "common/common_utils/StrictMode.hpp"
STRICT_MODE_OFF
#ifndef RPCLIB_MSGPACK
#define RPCLIB_MSGPACK clmdep_msgpack
#endif // !RPCLIB_MSGPACK
// #include "rpc/rpc_error.h"
STRICT_MODE_ON

#include <iostream>
#include <chrono>
#include <thread>

#include <glib.h>
#include <gst/app/app.h>
#include <gst/gst.h>
#include "vehicles/multirotor/api/MultirotorRpcLibClient.hpp"

using namespace msr::airlib;
MultirotorRpcLibClient client;


static gboolean
bus_call (GstBus     *bus,
          GstMessage *msg,
          gpointer    data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;

    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);

      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }

  return TRUE;
}


typedef struct _PipelineData {
  GstElement *pipeline, *app_source, *sink_udp, *queue_0, *convert, *enc_h264, *enc_rtp;
  GMainLoop *main_loop;  /* GLib's Main Loop */

  int image_width, image_height;
} PipelineData;


static int runGstreamer(int *argc, char **argv[], PipelineData *data, int width,
int height, int framerate, int port) {
    GstBus *bus;
    guint bus_watch_id;

    // initialize gstreamer
    gst_init(argc, argv);

    // Create the elements
    data->app_source = gst_element_factory_make("appsrc", "video_source");
    data->queue_0 = gst_element_factory_make("queue", "queue_0");
    data->convert = gst_element_factory_make("videoconvert", "convert");
    data->enc_h264 = gst_element_factory_make("x264enc", "enc_h264");
    data->enc_rtp = gst_element_factory_make("rtph264pay", "enc_rtp");
    data->sink_udp = gst_element_factory_make("udpsink", "sink_udp");

    // create empty pipeline
    data->pipeline = gst_pipeline_new("video-pipeline");

    if (!data->pipeline || !data->app_source || !data->sink_udp || !data->convert
        || !data->enc_h264 || !data->enc_rtp) {
        g_printerr("Not all elements could be created\n");
        g_print("\npipeline: ");
        std::cout << data->pipeline;
        g_print("\napp_source: ");
        std::cout << data->app_source;
        g_print("\nsink_udp: ");
        std::cout << data->sink_udp;
        g_print("\nqueue_0: ");
        std::cout << data->queue_0;
        g_print("\nconvert: ");
        std::cout << data->convert;
        g_print("\nenc_h264: ");
        std::cout << data->enc_h264;
        g_print("\nenc_rtp: ");
        std::cout << data->enc_rtp;

        return -1;
    }

    // element configuration goes here
    g_object_set(G_OBJECT(data->app_source),
                "format", 3,
                "is-live", true,
                NULL);
    g_object_set (G_OBJECT (data->sink_udp), "host", "localhost", NULL);
    g_object_set (G_OBJECT (data->sink_udp), "port", 5000, NULL);
    g_object_set (G_OBJECT (data->enc_h264), "bitrate", 500, NULL);
    g_object_set (G_OBJECT (data->enc_h264), "tune", 0x00000004, NULL);
    g_object_set (G_OBJECT (data->enc_h264), "speed-preset", 2, NULL);

    // link elements
    gst_bin_add_many(
        GST_BIN (data->pipeline),
        data->app_source,
        data->queue_0,
        data->convert,
        data->enc_h264,
        data->enc_rtp,
        data->sink_udp,
        NULL);
    
    GstCaps *caps_source;
    caps_source = gst_caps_new_simple("video/x-raw",
            "format", G_TYPE_STRING, "BGR",
            "framerate", GST_TYPE_FRACTION, framerate, 1,
            "width", G_TYPE_INT, width,
            "height", G_TYPE_INT, height,
            "bpp", G_TYPE_INT, 24,
            "depth", G_TYPE_INT, 8,
            NULL);

    if (!gst_element_link_filtered(data->app_source, data->queue_0, caps_source)) {
        g_printerr("Elements app_source and queue_0 could not be linked.\n");
        gst_object_unref(data->pipeline);
        return -1;
    }
    gst_caps_unref(caps_source);

    if (!gst_element_link(data->queue_0, data->convert)) {
        g_printerr("Elements queue_0 and convert could not be linked.\n");
        gst_object_unref(data->pipeline);
        return -1;
    }


    // encode from BGR to I420 because nvvidconv on receiving side doesn't receive BGR
    GstCaps *caps_convert;
    caps_convert = gst_caps_new_simple("video/x-raw",
            "format", G_TYPE_STRING, "I420",
            NULL);

    if (!gst_element_link_filtered(data->convert, data->enc_h264, caps_convert)) {
        g_printerr("Elements convert and enc_h264 could not be linked.\n");
        gst_object_unref(data->pipeline);
        return -1;
    }
    gst_caps_unref(caps_convert);
    

    if (gst_element_link_many(data->enc_h264, data->enc_rtp, data->sink_udp, NULL) != TRUE) {
        g_printerr("Elements enc_h264 through sink_udp could not be linked.\n");
        gst_object_unref(data->pipeline);
        return -1;
    }

    // start playing the pipeline
    gst_element_set_state(data->pipeline, GST_STATE_PLAYING);

    // create and start main loop
    data->main_loop = g_main_loop_new(NULL, FALSE);

    // add a message handler
    bus = gst_pipeline_get_bus(GST_PIPELINE (data->pipeline));
    bus_watch_id = gst_bus_add_watch(bus, bus_call, data->main_loop);
    gst_object_unref(bus);

    g_main_loop_run(data->main_loop);

    // free resources
    g_print ("Returned, stopping playback\n");
    gst_element_set_state(data->pipeline, GST_STATE_NULL);

    g_print ("Deleting pipeline\n");
    gst_object_unref(GST_OBJECT(data->pipeline));
    g_source_remove(bus_watch_id);
    g_main_loop_unref(data->main_loop);
    return 0;
}


static ImageCaptureBase::ImageResponse getOneImage() {
    // getImages provides more details about the image
    std::vector<ImageCaptureBase::ImageRequest> request = {
        ImageCaptureBase::ImageRequest(
            "front_center",
            ImageCaptureBase::ImageType::Scene,
            false,
            false
        )
    };
    
    return client.simGetImages(request)[0];
}


static void sendImageStream(PipelineData * pipelineData, int fps) {
    while(1) {
        ImageCaptureBase::ImageResponse new_image = getOneImage();
        pipelineData->image_width = new_image.width;
        pipelineData->image_height = new_image.height;
        
        // check that appsrc element is created in gstreamer thread before using
        if (pipelineData->app_source) {
            GstBuffer *buffer;
            GstMapInfo map;
            GstFlowReturn ret;
            
            // create buffer and allocate memory
            buffer = gst_buffer_new_allocate(NULL, (gsize)(new_image.image_data_uint8.size()), NULL);
            // fill writable map with (ideally writable) memory blocks in the buffer
            gst_buffer_map(buffer, &map, GST_MAP_WRITE);
            memcpy(map.data, new_image.image_data_uint8.data(), new_image.image_data_uint8.size());
            map.size = new_image.image_data_uint8.size();
            ret = gst_app_src_push_buffer(GST_APP_SRC(pipelineData->app_source), buffer);
            // release buffer memory that was associated with map
            gst_buffer_unmap(buffer, &map);
            // see flow error type of GstFlowReturn
            if (ret != 0) {
                g_print("\nPush appsrc buffer flow error: %d\n", ret);
            }
        }
        else {
            g_print("AppSrc element not yet created - image skipped\n");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds((int)((1 / (float) fps) * 1e3)));
    }
}


int main(int argc, char *argv[]) {
    int framerate = 15;
    int framerate_input = 0;
    int port = 5000;
    for (int i=0; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss >> framerate_input)) {
                std::cerr << "Invalid framerate: " << argv[i + 1] << '\n';
            } else if (!ss.eof()) {
                std::cerr << "Trailing characters after framerate: " << argv[i + 1] << '\n';
            }
        }

        if (strcmp(argv[i], "-p") == 0) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss >> port)) {
                std::cerr << "Invalid port: " << argv[i + 1] << '\n';
            } else if (!ss.eof()) {
                std::cerr << "Trailing characters after port: " << argv[i + 1] << '\n';
            }
        }
    }
    
    if (framerate_input != 0 && framerate_input <= 60) {
        framerate = framerate_input;
        std::cout << "Framerate set to: " << framerate << " fps" << std::endl;
    } else {
        std::cout << "Framerate set to: " << framerate << " fps (Default)" << std::endl;
    }

    if (port == 5000) {
        std::cout << "Port set to: " << port << " (Default)" << std::endl;
    } else {
        std::cout << "Port set to: " << port << std::endl;
    }

    PipelineData data = {};
    
    std::thread feedAppSrc(sendImageStream, &data, framerate);

    // wait for sendImageStream to get image data from AirSim
    while (1) {
        if (data.image_width != 0 && data.image_height != 0) {
            std::cout << "Image width: " << data.image_width << ", height: " << data.image_height << std::endl;
            int pipeline_status = runGstreamer(&argc, &argv, &data, data.image_width,
            data.image_height, framerate, port);

            if (!pipeline_status) {
                feedAppSrc.join();
            }

            if (pipeline_status) {
                std::cout << "\nPipeline failed to run: terminating feedAppSrc and the program" << std::endl;
            }

            return pipeline_status;
        }
    }
}

