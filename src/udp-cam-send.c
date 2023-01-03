#include <gst/gst.h>
#include <glib.h>


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


int
main (int   argc,
      char *argv[])
{
  GMainLoop *loop;

  GstElement *pipeline, *source, *demux, *conv, *sink, *h264enc, *rtpenc, *scale;
  GstBus *bus;
  guint bus_watch_id;

  /* Initialisation */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);


  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("video-sender");
  source   = gst_element_factory_make ("avfvideosrc",   "video-src");
  scale   = gst_element_factory_make ("videoscale",   "video-scale");
  conv     = gst_element_factory_make ("videoconvert",  "converter");
  h264enc   = gst_element_factory_make ("x264enc",  "h264-encode");
  // h264enc   = gst_element_factory_make ("vtenc_h264_hw",  "h264-encode");
  rtpenc   = gst_element_factory_make ("rtph264pay",  "rtp-encode");
  sink     = gst_element_factory_make ("udpsink", "udp-output");

  if (!pipeline || !source || !conv || !sink || !h264enc || !rtpenc || !scale) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  /* Set up the pipeline */

  /* we set the input filename to the source element */
  g_object_set (G_OBJECT (source), "device-index", 1, NULL);
  g_object_set (G_OBJECT (sink), "host", "192.168.2.2", NULL);
  g_object_set (G_OBJECT (sink), "port", 5000, NULL);
  // g_object_set (G_OBJECT (sink), "async", 0, NULL);
  g_object_set (G_OBJECT (h264enc), "bitrate", 500, NULL);
  g_object_set (G_OBJECT (h264enc), "tune", 0x00000004, NULL);
  g_object_set (G_OBJECT (h264enc), "speed-preset", 2, NULL);

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* we add all elements into the pipeline */
  /* video-src | WAV-demux | converter | alsa-output */
  gst_bin_add_many (GST_BIN (pipeline),
                    source, scale, conv, h264enc, rtpenc, sink, NULL);


  /* 
  appears may need to add caps for udpsink 
  ‘application/x-rtp, encoding-name=H264, payload=96’
  https://gstreamer.freedesktop.org/documentation/application-development/basics/pads.html?gi-language=c
  */
  gboolean link_ok;
  // GstCaps *caps;

  // caps = gst_caps_new_simple ("application/x-rtp",
  //         "encoding-name", G_TYPE_STRING, "H264",
  //         "payload", G_TYPE_INT, 96,
  //         "clock-rate", G_TYPE_INT, 90000,
  //         "media", G_TYPE_STRING, "video",
  //         NULL);

  // link_ok = gst_element_link_filtered (rtpenc, sink, caps);
  // gst_caps_unref (caps);

  // if (!link_ok) {
  //   g_warning ("Failed to link rtpenc and sink!");
  // }

  GstCaps *caps_scale;

  caps_scale = gst_caps_new_simple ("video/x-raw",
          "width", G_TYPE_INT, 640,
          "height", G_TYPE_INT, 480,
          NULL);

  link_ok = gst_element_link_filtered (source, scale, caps_scale);
  gst_caps_unref (caps_scale);

  if (!link_ok) {
    g_warning ("Failed to link source and scale!");
  }

  gboolean link_many_ok;
  /* we link the elements together */
  /* video-src -> WAV-demux -> converter -> auto-output */
  link_many_ok = gst_element_link_many (scale, conv, h264enc, rtpenc, sink, NULL);
  if (!link_many_ok) {
    g_warning ("Failed to link remaining elements!");
  }

  /* Set the pipeline to "playing" state*/
  gst_element_set_state (pipeline, GST_STATE_PLAYING);


  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);


  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);

  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);

  return 0;
}