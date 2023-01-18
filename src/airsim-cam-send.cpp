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
  GstElement *pipeline, *app_source, *app_sink;

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
    data->app_sink = gst_element_factory_make ("fakesink", "video_sink");

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

        return -1;
    }

    // element configuration goes here
    g_object_set(G_OBJECT(data->app_sink), "dump", TRUE, NULL); // dump data reveived by fakesink to stdout

    // link elements
    gst_bin_add_many(GST_BIN (data->pipeline), data->app_source, data->app_sink, NULL);
    if (gst_element_link_many (data->app_source, data->app_sink, NULL) != TRUE) {
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
    return client.simGetImage("front_center", ImageCaptureBase::ImageType::Scene);
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
            map.data = newImage.data();
            // release buffer memory that was associated with map
            gst_buffer_unmap(buffer, &map);
            ret = gst_app_src_push_buffer(GST_APP_SRC(pipelineData->app_source), buffer);
            // TODO: utilize return to exit program on failure and indicate failure
        }
        else {
            std::cout << "AppSrc element not yet created - image skipped" << std::endl;
        }
        std::cout << "Image unit8 size: " << newImage.size() << std::endl;
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

