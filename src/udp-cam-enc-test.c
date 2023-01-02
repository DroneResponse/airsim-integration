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

  GstElement *pipeline, *source, *demux, *conv, *conv1, *sink, *rtpdec, *h264dec, *rtpenc, *h264enc, *queue;
  GstBus *bus;
  guint bus_watch_id;

  /* Initialisation */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);


  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("video-player");
  source   = gst_element_factory_make ("avfvideosrc",   "video-src");
  conv     = gst_element_factory_make ("videoconvert",  "converter");
  conv1     = gst_element_factory_make ("videoconvert",  "converter-1");
  h264enc   = gst_element_factory_make ("vtenc_h264_hw",  "h264-encode");
  queue = gst_element_factory_make ("queue",  "queue");
  h264dec   = gst_element_factory_make ("vtdec_hw",  "h264-decode");
//   h264dec   = gst_element_factory_make ("decodebin",  "h264-decode");
  sink     = gst_element_factory_make ("autovideosink", "video-output");

  if (!pipeline || !source || !conv || !conv1 || !sink || !h264dec || !h264enc || !queue) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  /* Set up the pipeline */
  g_object_set (G_OBJECT (source), "device-index", 0, NULL);
  // g_object_set (G_OBJECT (h264enc), "bitrate", 1000, NULL);
  g_object_set (G_OBJECT (h264enc), "realtime", 1, NULL);

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* we add all elements into the pipeline */
  gst_bin_add_many (GST_BIN (pipeline),
                    source, h264enc, conv, conv1, sink, h264dec, queue, NULL);
  
  gboolean link_many_ok;
  /* we link the elements together */
  /* video-src -> WAV-demux -> converter -> auto-output */
  link_many_ok = gst_element_link_many (source, conv, h264enc, queue, h264dec, conv1, sink, NULL);

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