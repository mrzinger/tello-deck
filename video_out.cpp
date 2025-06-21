#include "video_out.h"
#include <stdlib.h>
#include <isl/map.h>

const char *VideoOut::g_def_label[MAX_LBL] = {"📶Wifi:", "↑Alt:", "🔋Bat:"};

VideoOut::VideoOut() : pipeline(NULL), video_window_handle(0) {
    for (int i = 0; i < MAX_LBL; ++i) g_video_lbls[i] = NULL;
}

void VideoOut::start_video()
{
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

GstBusSyncReply VideoOut::bus_sync_handler(GstBus *bus, GstMessage *message, gpointer user_data)
{
    VideoOut *self = static_cast<VideoOut*>(user_data);
    if (!gst_is_video_overlay_prepare_window_handle_message(message))
        return GST_BUS_PASS;

    if (self->video_window_handle != 0)
    {
        GstVideoOverlay *overlay = GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message));
        gst_video_overlay_set_window_handle(overlay, self->video_window_handle);
    }
    else
    {
        g_warning("Should have obtained video_window_handle by now!");
    }

    gst_message_unref(message);
    return GST_BUS_DROP;
}

void VideoOut::realize_cb(GtkWidget *widget, gpointer data)
{
    VideoOut *self = static_cast<VideoOut*>(data);
    GdkWindow *window = gtk_widget_get_window(widget);

    if (!gdk_window_ensure_native(window))
        g_error("Couldn't create native window needed for GstVideoOverlay");

    self->video_window_handle = GDK_WINDOW_XID(window);
}

gboolean VideoOut::delete_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    VideoOut *self = static_cast<VideoOut*>(data);
    gst_element_set_state(self->pipeline, GST_STATE_NULL);
    gst_object_unref(self->pipeline);
    return FALSE;
}

void VideoOut::gst_pad_link_elements(GstElement *element, GstPad *pad, gpointer data)
{
    GstElement *other_element = static_cast<GstElement*>(data);
    GstPad *sinkpad = gst_element_get_static_pad(other_element, "sink");
    if (gst_pad_link(pad, sinkpad) != GST_PAD_LINK_OK)
        g_printerr("Failed to link elements.\n");
    gst_object_unref(sinkpad);
}

void VideoOut::init_video_screen(GtkWidget *video_screen_parent)
{
    gst_init(NULL, NULL);

    GstElement *udpsrc = gst_element_factory_make("udpsrc", "udpsrc");
    GstElement *decodebin = gst_element_factory_make("decodebin", "decodebin");
    GstElement *queue = gst_element_factory_make("queue", "queue");
    GstElement *videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    GstElement *sink = gst_element_factory_make("autovideosink", "autovideosink");

    g_object_set(G_OBJECT(udpsrc), "uri", "udp://0.0.0.0:11111", NULL);
    g_object_set(G_OBJECT(queue), "max-size-buffers", 1, "max-size-time", 0, "max-size-bytes", 0, NULL);
    g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);

    GstElement *wifi_label = gst_element_factory_make("textoverlay", "textoverlay1");
    GstElement *alt_label = gst_element_factory_make("textoverlay", "textoverlay2");
    GstElement *bat_label = gst_element_factory_make("textoverlay", "textoverlay3");

    const char *font = "Hack, 10";
    g_object_set(G_OBJECT(wifi_label), "name", "wifi_label", "font-desc", font,
                 "text", g_def_label[WIFI_LBL], "valignment", 2, "halignment", 2, NULL);
    g_object_set(G_OBJECT(alt_label), "name", "altitude_label", "font-desc", font,
                 "text", g_def_label[ALT_LBL], "valignment", 2, "halignment", 1, NULL);
    g_object_set(G_OBJECT(bat_label), "name", "batery_label", "font-desc", font,
                 "text", g_def_label[BAT_LBL], "valignment", 2, "halignment", 0, NULL);

    g_video_lbls[WIFI_LBL] = wifi_label;
    g_video_lbls[ALT_LBL] = alt_label;
    g_video_lbls[BAT_LBL] = bat_label;

    pipeline = gst_pipeline_new("pipeline");

    gst_bin_add_many(GST_BIN(pipeline), udpsrc, decodebin, queue, videoconvert,
                     wifi_label, alt_label, bat_label, sink, NULL);

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

    g_signal_connect(decodebin, "pad-added", G_CALLBACK(VideoOut::gst_pad_link_elements), queue);

    g_signal_connect(video_screen_parent, "realize", G_CALLBACK(VideoOut::realize_cb), this);
    g_signal_connect(video_screen_parent, "delete-event", G_CALLBACK(VideoOut::delete_event_cb), this);

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_set_sync_handler(bus, (GstBusSyncHandler)VideoOut::bus_sync_handler, this, NULL);
    gst_object_unref(bus);
}

void VideoOut::set_label(VideoLabels label_id, const char *text)
{
    char label[64];
    sprintf(label, "%s %s", g_def_label[label_id], text);
    g_object_set(G_OBJECT(g_video_lbls[label_id]), "text", label, NULL);
}
