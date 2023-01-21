// compile only working: g++ -std=c++0x -c ./src/airsim-cam-send.cpp -o airsim-cam-send -I/Users/jasonbrauer/drone_response/AirSim/AirLib/include -I/Users/jasonbrauer/drone_response/AirSim/AirLib/deps/eigen3
// w/ linking failing: g++ -std=c++0x ./src/airsim-cam-send.cpp -o airsim-cam-send -I/Users/jasonbrauer/drone_response/AirSim/AirLib/include -I/Users/jasonbrauer/drone_response/AirSim/AirLib/deps/eigen3 -L/Users/jasonbrauer/drone_response/AirSim/build_release/output/lib

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
  GstElement *pipeline, *app_source, *app_sink, *video_convert, *video_raw_parse, *queue_0;

//   guint64 num_samples;   /* Number of samples generated so far (for timestamp generation) */

  GMainLoop *main_loop;  /* GLib's Main Loop */
} PipelineData;


static int runGstreamer(int *argc, char **argv[], PipelineData *data) {
    GstBus *bus;
    guint bus_watch_id;

    // initialize gstreamer
    gst_init(argc, argv);

    // Create the elements
    data->app_source = gst_element_factory_make ("appsrc", "video_source");
    data->queue_0 = gst_element_factory_make ("queue", "queue_0");
    data->video_raw_parse = gst_element_factory_make ("rawvideoparse", "video_raw_parse");
    data->video_convert = gst_element_factory_make ("videoconvert", "video_convert");
    data->app_sink = gst_element_factory_make ("autovideosink", "video_sink");

    // create empty pipeline
    data->pipeline = gst_pipeline_new ("video-pipeline");

    if (!data->pipeline || !data->app_source || !data->app_sink) {
        g_printerr("Not all elements could be created\n");
        g_print("\npipeline: ");
        std::cout << data->pipeline;
        g_print("\napp_source: ");
        std::cout << data->app_source;
        g_print("\napp_sink: ");
        std::cout << data->app_sink;
        g_print("\nvideo_convert: ");
        std::cout << data->video_convert;
        g_print("\nvideo_raw_parse: ");
        std::cout << data->video_raw_parse;
        g_print("\nqueue_0: ");
        std::cout << data->queue_0;

        return -1;
    }

    // element configuration goes here
    // g_object_set(G_OBJECT(data->app_sink), "dump", TRUE, NULL); // dump data reveived by fakesink to stdout
    // g_object_set(G_OBJECT(data->png_dec), "output-corrupt", TRUE, NULL);
    // GValue plane_offsets = G_VALUE_INIT;
    // g_value_init (&plane_offsets, GST_TYPE_ARRAY);
    // GValue val = G_VALUE_INIT;
    // g_value_init (&val, G_TYPE_INT);

    // g_value_set_int(&val, (gint)256);
    // gst_value_array_append_value(&plane_offsets, &val);
    // gst_value_array_append_value(&plane_offsets, &val);

    // g_object_set_property (G_OBJECT (data->video_raw_parse), "plane-offsets", &plane_offsets);

    g_object_set(G_OBJECT(data->video_raw_parse), 
                        "format", 15,
                        "framerate", 30, 1,
                        "width", 256,
                        "height", 144,
                        NULL);

    // link elements
    gst_bin_add_many(
        GST_BIN (data->pipeline),
        data->app_source,
        data->queue_0,
        data->video_raw_parse,
        data->video_convert,
        data->app_sink,
        NULL);
    
    GstCaps *caps_source;
    // TODO: set these caps dynamically based on what AirSim is returning in image response
    // and the fps set in main
    caps_source = gst_caps_new_simple ("video/x-unaligned-raw",
            "format", G_TYPE_STRING, "RGB",
            "framerate", GST_TYPE_FRACTION, 30, 1,
            "width", G_TYPE_INT, 256,
            "height", G_TYPE_INT, 144,
            NULL);

    if (!gst_element_link_filtered(data->app_source, data->queue_0, caps_source)) {
        g_printerr("Elements app_source and queue_0 could not be linked.\n");
        gst_object_unref (data->pipeline);
        return -1;
    }
    gst_caps_unref(caps_source);

    if (!gst_element_link(data->queue_0, data->video_raw_parse)) {
        g_printerr("Elements queue_0 and video_raw_parse could not be linked.\n");
        gst_object_unref (data->pipeline);
        return -1;
    }

    GstCaps *caps_parse;
    // TODO: set these caps dynamically based on what AirSim is returning in image response
    // and the fps set in main
    caps_parse = gst_caps_new_simple ("video/x-raw",
            "format", G_TYPE_STRING, "RGB",
            "framerate", GST_TYPE_FRACTION, 30, 1,
            "width", G_TYPE_INT, 256,
            "height", G_TYPE_INT, 144,
            NULL);

    if (!gst_element_link_filtered(data->video_raw_parse, data->video_convert, caps_parse)) {
        g_printerr("Elements video_raw_parse and video_convert could not be linked.\n");
        gst_object_unref (data->pipeline);
        return -1;
    }
    gst_caps_unref(caps_parse);
    
    if (gst_element_link_many (data->video_convert, data->app_sink, NULL) != TRUE) {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref (data->pipeline);
        return -1;
    }

    // start playing the pipeline
    gst_element_set_state (data->pipeline, GST_STATE_PLAYING);

    // create and start main loop
    // add a message handler
    data->main_loop = g_main_loop_new (NULL, FALSE);

    bus = gst_pipeline_get_bus (GST_PIPELINE (data->pipeline));
    bus_watch_id = gst_bus_add_watch (bus, bus_call, data->main_loop);
    gst_object_unref (bus);

    g_main_loop_run (data->main_loop);

    // Free resources
    gst_element_set_state (data->pipeline, GST_STATE_NULL);
    gst_object_unref (data->pipeline);
    return 0;
}


static vector<uint8_t> getOneImage() {
    // getImages provides more details about the image
    std::vector<ImageCaptureBase::ImageRequest> request = {
        ImageCaptureBase::ImageRequest(
            "front_center",
            ImageCaptureBase::ImageType::Scene,
            false,
            false
        )
    };
    
    ImageCaptureBase::ImageResponse imageResponse = client.simGetImages(request)[0];
    // g_print("\nImage Width: %d Height: %d", imageResponse.width, imageResponse.height);
    return imageResponse.image_data_uint8;

    // return client.simGetImage("front_center", ImageCaptureBase::ImageType::Scene);
}


static void sendImageStream(PipelineData * pipelineData, int fps) {
    printf("Milliseconds between frames: %d\n", (int)((1 / (float) fps) * 1e3));

    while(1) {
        vector<uint8_t> newImage = getOneImage();
        
        // check that appsrc element is created in gstreamer thread before using
        if (pipelineData->app_source) {
            GstBuffer *buffer;
            GstMapInfo map;
            GstFlowReturn ret;
            
            // create buffer and allocate memory
            buffer = gst_buffer_new_allocate(NULL, (gint)newImage.size(), NULL);
            // fill writable map with (ideally writable) memory blocks in the buffer
            gst_buffer_map(buffer, &map, GST_MAP_WRITE);
            // newImage.shrink_to_fit();
            std::rotate(newImage.rbegin(), newImage.rbegin() + 150, newImage.rend());
            map.data = newImage.data();
            // release buffer memory that was associated with map
            gst_buffer_unmap(buffer, &map);
            ret = gst_app_src_push_buffer(GST_APP_SRC(pipelineData->app_source), buffer);
            // see flow error type of GstFlowReturn
            if (ret != 0) {
                g_print("\nPush appsrc buffer flow error: %d\n", ret);
            }
        }
        else {
            std::cout << "AppSrc element not yet created - image skipped" << std::endl;
        }
        // std::cout << "\nImage unit8 size: " << newImage.size() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds((int)((1 / (float) fps) * 1e3)));
    }
}


int main(int argc, char *argv[]) {
    PipelineData data = {};
    
    std::thread feedAppSrc(sendImageStream, &data, 30);

    int pipelineStatus = runGstreamer(&argc, &argv, &data);

    if (!pipelineStatus) {
        feedAppSrc.join();
    }

    if (pipelineStatus) {
        std::cout << "\nPipeline failed to run: terminating feedAppSrc and the program" << std::endl;
    }

    return pipelineStatus;
}

