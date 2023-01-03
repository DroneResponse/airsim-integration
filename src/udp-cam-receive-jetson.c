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

  GstElement *pipeline, *source, *demux, *conv, *sink, *rtpdec, *h264dec, *h264parse;
  GstBus *bus;
  guint bus_watch_id;

  /* Initialisation */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);


  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("video-player");
  source   = gst_element_factory_make ("udpsrc", "udp-input");
  conv     = gst_element_factory_make ("videoconvert",  "converter");
  rtpdec   = gst_element_factory_make ("rtph264depay",  "rtp-decode");
//   h264dec   = gst_element_factory_make ("vtdec_hw",  "h264-decode");
  h264dec   = gst_element_factory_make ("omxh264dec",  "h264-decode");
  h264parse   = gst_element_factory_make ("h264parse",  "h264-parse");
//   h264dec   = gst_element_factory_make ("decodebin",  "h264-decode");
  sink     = gst_element_factory_make ("ximagesink", "video-output");
//   sink     = gst_element_factory_make ("fakesink", "video-output");

  if (!pipeline || !source || !conv || !sink || !rtpdec || !h264dec || !h264parse) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  /* Set up the pipeline */

  /* we set the input filename to the source element */
  g_object_set (G_OBJECT (source), "port", 5000, NULL);

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* we add all elements into the pipeline */
  /* video-src | WAV-demux | converter | alsa-output */
  gst_bin_add_many (GST_BIN (pipeline),
                    source, rtpdec, h264dec, conv, sink, h264parse, NULL);

  gboolean link_ok;
  GstCaps *caps;

  caps = gst_caps_new_simple ("application/x-rtp",
          "encoding-name", G_TYPE_STRING, "H264",
          "payload", G_TYPE_INT, 96,
          "clock-rate", G_TYPE_INT, 90000,
          "media", G_TYPE_STRING, "video",
          NULL);

  link_ok = gst_element_link_filtered (source, rtpdec, caps);
  gst_caps_unref (caps);

  if (!link_ok) {
    g_warning ("Failed to link rtpenc and sink!");
  }
  
  gboolean link_many_ok;
  /* we link the elements together */
  /* video-src -> WAV-demux -> converter -> auto-output */
  link_many_ok = gst_element_link_many (rtpdec, h264parse, h264dec, conv, sink, NULL);
//   link_many_ok = gst_element_link_many(rtpdec, sink, NULL);

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