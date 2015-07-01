#include <stdio.h>
#include <glib.h>
#include <gst/gst.h>


static void
on_pad_added (GstElement *element,
	      GstPad *pad,
	      gpointer data)
{
  GstPad *sinkpad;
  GstElement *decoder = (GstElement *) data;
  g_print ("Dynamic pad created\n");

  sinkpad = gst_element_get_static_pad(decoder, "sink");
  gst_pad_link(pad, sinkpad);
  gst_object_unref(sinkpad);
}

static gboolean
bus_call (GstBus *bus,
	  GstMessage *msg,
	  gpointer data)
{
  GMainLoop *loop = (GMainLoop *)data;

  switch (GST_MESSAGE_TYPE (msg))
    {
    case GST_MESSAGE_EOS:
      g_print("End of stream\n");
      g_main_loop_quit(loop);
      break;
    case GST_MESSAGE_ERROR: {
      gchar *debug;
      GError *error;
      gst_message_parse_error(msg, &error, &debug);
      g_free(debug);
      g_printerr("Error: %s\n", error->message);
      g_error_free(error);
      g_main_loop_quit(loop);
      break;
    }
    default:
      break;
    }

    return TRUE;
}

int
main(int argc, char *argv[])
{
  GMainLoop *loop;
  GstElement *pipeline, *source, *demuxer, *decoder, *conv, *sink;
  GstBus *bus;
  guint bus_watch_id;
  
  gst_init(&argc, &argv);

  loop = g_main_loop_new(NULL, FALSE);

  // gstreamer elements
  pipeline = gst_pipeline_new("audio-player");
  source	= gst_element_factory_make("filesrc"		, "file-source");
  demuxer	= gst_element_factory_make("oggdemux"		, "ogg-demuxer");
  decoder	= gst_element_factory_make("vorbisdec"		, "vorbis-decoder");
  conv		= gst_element_factory_make("audioconvert"	, "converter");
  sink		= gst_element_factory_make("autoaudiosink"	, "audio-output");

  if (!pipeline || !source || !demuxer || !decoder || !conv || !sink)
    {
      g_printerr("Some element could not be created.\n");
      return -1;
    }

  // set up the pipeline
  // 1 - Source element
  g_object_set(G_OBJECT (source), "location", argv[1], NULL);

  // message handler
  bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  bus_watch_id = gst_bus_add_watch(bus, bus_call, loop);

  // add all the elements to the pipeline
  gst_bin_add_many(GST_BIN(pipeline),
		   source, demuxer, decoder, conv, sink,
		   NULL);
  // now link them
  gst_element_link(source, demuxer);
  gst_element_link_many(decoder, conv, sink, NULL);
  g_signal_connect(demuxer, "pad-added", G_CALLBACK(on_pad_added), decoder);

  g_print("Now playing: %s\n", argv[1]);
  gst_element_set_state(pipeline, GST_STATE_PLAYING);
  
  // run the main loop
  g_print("Running...\n");
  g_main_loop_run(loop);
  g_print("Returned from main loop.\n");

  return 0;
}


