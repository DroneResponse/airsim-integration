#include "common/common_utils/StrictMode.hpp"
STRICT_MODE_OFF
#ifndef RPCLIB_MSGPACK
#define RPCLIB_MSGPACK clmdep_msgpack
#endif // !RPCLIB_MSGPACK
// #include "rpc/rpc_error.h"
STRICT_MODE_ON

#include "vehicles/multirotor/api/MultirotorRpcLibClient.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <gst/gst.h>
#include <gst/app/app.h>
#include <glib.h>

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
  GstElement *pipeline, *app_source, *app_sink, *queue_0;
  GMainLoop *main_loop;  /* GLib's Main Loop */

  int image_width, image_height;
} PipelineData;


static int runGstreamer(int *argc, char **argv[], PipelineData *data, int width,
int height, int framerate) {
    GstBus *bus;
    guint bus_watch_id;

    // initialize gstreamer
    gst_init(argc, argv);

    // Create the elements
    data->app_source = gst_element_factory_make("appsrc", "video_source");
    data->queue_0 = gst_element_factory_make("queue", "queue_0");
    data->app_sink = gst_element_factory_make("autovideosink", "video_sink");

    // create empty pipeline
    data->pipeline = gst_pipeline_new("video-pipeline");

    if (!data->pipeline || !data->app_source || !data->app_sink) {
        g_printerr("Not all elements could be created\n");
        g_print("\npipeline: ");
        std::cout << data->pipeline;
        g_print("\napp_source: ");
        std::cout << data->app_source;
        g_print("\napp_sink: ");
        std::cout << data->app_sink;
        g_print("\nqueue_0: ");
        std::cout << data->queue_0;

        return -1;
    }

    // element configuration goes here
    g_object_set(G_OBJECT(data->app_source),
                "format", 3,
                "is-live", true,
                NULL);

    // link elements
    gst_bin_add_many(
        GST_BIN (data->pipeline),
        data->app_source,
        data->queue_0,
        data->app_sink,
        NULL);
    
    GstCaps *caps_source;
    // TODO: set these caps dynamically based on what AirSim is returning in image response
    // and the fps set in main
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
    
    if (gst_element_link_many(data->queue_0, data->app_sink, NULL) != TRUE) {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(data->pipeline);
        return -1;
    }

    // start playing the pipeline
    gst_element_set_state(data->pipeline, GST_STATE_PLAYING);

    // create and start main loop
    // add a message handler
    data->main_loop = g_main_loop_new(NULL, FALSE);

    bus = gst_pipeline_get_bus(GST_PIPELINE (data->pipeline));
    bus_watch_id = gst_bus_add_watch(bus, bus_call, data->main_loop);
    gst_object_unref(bus);

    g_main_loop_run(data->main_loop);

    // Free resources
    gst_element_set_state(data->pipeline, GST_STATE_NULL);
    gst_object_unref(data->pipeline);
    return 0;
}


static ImageCaptureBase::ImageResponse getOneImage(int frame) {
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
    printf("Milliseconds between frames: %d\n", (int)((1 / (float) fps) * 1e3));

    unsigned long frame_count = 1;
    while(1) {
        ImageCaptureBase::ImageResponse new_image = getOneImage(frame_count);
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
            std::cout << "AppSrc element not yet created - image skipped" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds((int)((1 / (float) fps) * 1e3)));

        frame_count++;
    }
}


int main(int argc, char *argv[]) {
    int framerate = 15;
    PipelineData data = {};
    
    std::thread feedAppSrc(sendImageStream, &data, framerate);

    while (1) {
        if (data.image_width != 0 && data.image_height != 0) {
            int pipeline_status = runGstreamer(&argc, &argv, &data, data.image_width,
            data.image_height, framerate);

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

