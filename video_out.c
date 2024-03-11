#include "video_out.h"
#include <stdlib.h>
#include <isl/map.h>


static GstElement *pipeline = NULL;
static GstElement *g_video_lbls[MAX_LBL];
static const char *g_def_label[MAX_LBL] = {"📶Wifi:", "↑Alt:", "🔋Bat:"};
static guintptr video_window_handle = 0;

void start_video()
{
    gst_element_set_state(pipeline, GST_STATE_PLAYING); // start showing displaying the video stream from camera
}

static GstBusSyncReply bus_sync_handler(GstBus *bus, GstMessage *message, gpointer user_data)
{
    // ignore anything but 'prepare-window-handle' element messages
    if (!gst_is_video_overlay_prepare_window_handle_message(message))
        return GST_BUS_PASS;

    if (video_window_handle != 0)
    {
        GstVideoOverlay *overlay;

        // GST_MESSAGE_SRC (message) will be the video sink element
        overlay = GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message));
        gst_video_overlay_set_window_handle(overlay, video_window_handle);
    }
    else
    {
        g_warning("Should have obtained video_window_handle by now!");
    }

    gst_message_unref(message);
    return GST_BUS_DROP;
}

static void realize_cb(GtkWidget *widget, gpointer data)
{
    GdkWindow *window = gtk_widget_get_window(widget);

    if (!gdk_window_ensure_native(window))
        g_error("Couldn't create native window needed for GstVideoOverlay");

    video_window_handle = GDK_WINDOW_XID(window);
}

static gboolean delete_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    // Set the pipeline state to NULL
    gst_element_set_state(pipeline, GST_STATE_NULL);

    // Unref the pipeline
    gst_object_unref(pipeline);

    // Continue with the default delete-event handler
    return FALSE;
}

static void gst_pad_link_elements(GstElement *element, GstPad *pad, gpointer data) {
  GstElement *other_element = (GstElement *)data;
  GstPad *sinkpad;

  sinkpad = gst_element_get_static_pad(other_element, "sink");
  if (gst_pad_link(pad, sinkpad) != GST_PAD_LINK_OK) {
    g_printerr("Failed to link elements.\n");
  }
  gst_object_unref(sinkpad);
}

void init_video_screen(GtkWidget *video_screen_parent)
{
    // Initialize GStreamer
    gst_init(NULL, NULL);

    GstElement *udpsrc, *decodebin, *queue, *videoconvert, *sink;

    // Create elements
    udpsrc = gst_element_factory_make("udpsrc", "udpsrc");
    decodebin = gst_element_factory_make("decodebin", "decodebin");
    queue = gst_element_factory_make("queue", "queue");
    videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    sink = gst_element_factory_make("autovideosink", "autovideosink");

    // Set properties
    g_object_set(G_OBJECT(udpsrc), "uri", "udp://0.0.0.0:11111", NULL);
    g_object_set(G_OBJECT(queue), "max-size-buffers", 1, "max-size-time", 0, "max-size-bytes", 0, NULL);
    g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);

    GstElement *wifi_label, *alt_label, *bat_label;

    wifi_label = gst_element_factory_make("textoverlay", "textoverlay1");
    alt_label = gst_element_factory_make("textoverlay", "textoverlay2");
    bat_label = gst_element_factory_make("textoverlay", "textoverlay3");

    const char *font = "Hack, 10";
    g_object_set(G_OBJECT(wifi_label), "name", "wifi_label", "font-desc", font, "text", g_def_label[WIFI_LBL], "valignment", 2, "halignment", 2, NULL);
    g_object_set(G_OBJECT(alt_label), "name", "altitude_label", "font-desc", font, "text", g_def_label[ALT_LBL], "valignment", 2, "halignment", 1, NULL);
    g_object_set(G_OBJECT(bat_label), "name", "batery_label", "font-desc", font, "text", g_def_label[BAT_LBL], "valignment", 2, "halignment", 0, NULL);

    g_video_lbls[WIFI_LBL] = wifi_label;
    g_video_lbls[ALT_LBL] = alt_label;
    g_video_lbls[BAT_LBL] = bat_label;

    // Create pipeline
    pipeline = gst_pipeline_new("pipeline");

    // Add elements to the pipeline
    gst_bin_add_many(GST_BIN(pipeline), udpsrc, decodebin, queue, videoconvert, wifi_label, alt_label, bat_label, sink, NULL);

    // Link elements
    if (!gst_element_link(udpsrc, decodebin))
    {
        g_printerr("Failed to link udpsrc and decodebin.\n");
        return;
    }

    if (!gst_element_link_many(queue, videoconvert, wifi_label, alt_label, bat_label, sink, NULL))
    {
        g_printerr("Failed to link queue, videoconvert, and autovideosink.\n");
        return;
    }

    // Connect the "pad-added" signal
    g_signal_connect(decodebin, "pad-added", G_CALLBACK(gst_pad_link_elements), queue);

    g_signal_connect(video_screen_parent, "realize", G_CALLBACK(realize_cb), NULL);

    g_signal_connect(video_screen_parent, "delete-event", G_CALLBACK(delete_event_cb), NULL);

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_set_sync_handler(bus, (GstBusSyncHandler)bus_sync_handler, NULL, NULL);
    gst_object_unref(bus);
}

void set_label(VideoLabels label_id, const char *text)
{
    char label[64];
    sprintf(label, "%s %s", g_def_label[label_id], text);
    g_object_set(G_OBJECT(g_video_lbls[label_id]), "text", label, NULL);
}