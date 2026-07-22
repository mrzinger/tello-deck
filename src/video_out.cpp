#include "video_out.h"
#include <gdk/gdkx.h>
#include <stdlib.h>

const char *VideoOut::g_def_label[MAX_LBL] = {"📶Wifi:", "↑Alt:", "🔋Bat:"};

VideoOut::VideoOut() : pipeline(NULL), video_sink(NULL) {
    for (int i = 0; i < MAX_LBL; ++i) g_video_lbls[i] = NULL;
}

VideoOut::~VideoOut()
{
    if (pipeline == NULL) return;
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
}

void VideoOut::start_video()
{
    if (pipeline != NULL)
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void VideoOut::gst_pad_link_elements(GstElement *element, GstPad *pad, gpointer data)
{
    GstElement *other_element = static_cast<GstElement*>(data);
    GstPad *sinkpad = gst_element_get_static_pad(other_element, "sink");
    if (gst_pad_link(pad, sinkpad) != GST_PAD_LINK_OK)
        g_printerr("Failed to link elements.\n");
    gst_object_unref(sinkpad);
}

GstBusSyncReply VideoOut::bus_sync_handler(GstBus *bus, GstMessage *message, gpointer data)
{
    VideoOut *self = static_cast<VideoOut*>(data);
    if (!gst_is_video_overlay_prepare_window_handle_message(message))
        return GST_BUS_PASS;

    GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(g_object_get_data(
        G_OBJECT(self->video_sink), "video-widget")));
    if (window != NULL)
        gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message)),
                                            GDK_WINDOW_XID(window));

    gst_message_unref(message);
    return GST_BUS_DROP;
}

void VideoOut::realize_x11_video_widget(GtkWidget *widget, gpointer data)
{
    VideoOut *self = static_cast<VideoOut*>(data);
    GdkWindow *window = gtk_widget_get_window(widget);
    if (window == NULL || !gdk_window_ensure_native(window)) {
        g_warning("Could not create a native X11 window for video output");
        return;
    }

    g_object_set_data(G_OBJECT(self->video_sink), "video-widget", widget);
}

GtkWidget *VideoOut::init_video_screen()
{
    gst_init(NULL, NULL);

    GstElement *udpsrc = gst_element_factory_make("udpsrc", "udpsrc");
    GstElement *decodebin = gst_element_factory_make("decodebin", "decodebin");
    GstElement *queue = gst_element_factory_make("queue", "queue");
    GstElement *videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    const gboolean use_x11_sink = GDK_IS_X11_DISPLAY(gdk_display_get_default());
    GstElement *gtk_sink = NULL;
    GstElement *sink = NULL;
    GstElement *wifi_label = gst_element_factory_make("textoverlay", "textoverlay1");
    GstElement *alt_label = gst_element_factory_make("textoverlay", "textoverlay2");
    GstElement *bat_label = gst_element_factory_make("textoverlay", "textoverlay3");
    GstElement *overlay_blend = gst_element_factory_make("identity", "overlay_blend");

    if (use_x11_sink)
        sink = gst_element_factory_make("ximagesink", "ximagesink");
    else {
        gtk_sink = gst_element_factory_make("gtkglsink", "gtkglsink");
        sink = gst_element_factory_make("glsinkbin", "glsinkbin");
    }

    if (!udpsrc || !decodebin || !queue || !videoconvert || !sink ||
        (!use_x11_sink && !gtk_sink) ||
        !wifi_label || !alt_label || !bat_label || !overlay_blend) {
        g_warning("Could not create the GStreamer video elements");
        return gtk_label_new("Video output is unavailable");
    }

    GtkWidget *video_widget = NULL;
    if (use_x11_sink) {
        video_widget = gtk_drawing_area_new();
        g_signal_connect(video_widget, "realize",
                         G_CALLBACK(VideoOut::realize_x11_video_widget), this);
    } else {
        g_object_set(G_OBJECT(sink), "sink", gtk_sink, NULL);
        g_object_get(G_OBJECT(gtk_sink), "widget", &video_widget, NULL);
    }
    if (video_widget == NULL) {
        g_warning("Could not create the GTK video widget");
        return gtk_label_new("Video output is unavailable");
    }

    g_object_set(G_OBJECT(udpsrc), "uri", "udp://0.0.0.0:11111", NULL);
    g_object_set(G_OBJECT(queue), "max-size-buffers", 1, "max-size-time", 0,
                 "max-size-bytes", 0, "leaky", 2, NULL);
    g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);
    // GL sinks advertise GstVideoOverlayComposition support. Without an
    // allocation barrier, chained textoverlay elements can replace each
    // other's composition metadata instead of blending every label.
    g_object_set(G_OBJECT(overlay_blend), "drop-allocation", TRUE, NULL);
    video_sink = sink;

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
                     bat_label, wifi_label, alt_label, overlay_blend, sink, NULL);

    if (!gst_element_link(udpsrc, decodebin))
    {
        g_printerr("Failed to link udpsrc and decodebin.\n");
        return video_widget;
    }

    if (!gst_element_link_many(queue, videoconvert, bat_label, wifi_label, alt_label,
                               overlay_blend, sink, NULL))
    {
        g_printerr("Failed to link the video processing pipeline.\n");
        return video_widget;
    }

    g_signal_connect(decodebin, "pad-added", G_CALLBACK(VideoOut::gst_pad_link_elements), queue);
    if (use_x11_sink) {
        GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
        gst_bus_set_sync_handler(bus, VideoOut::bus_sync_handler, this, NULL);
        gst_object_unref(bus);
    }
    return video_widget;
}

void VideoOut::set_label(VideoLabels label_id, const char *text)
{
    if (g_video_lbls[label_id] == NULL) return;
    char label[64];
    sprintf(label, "%s %s", g_def_label[label_id], text);
    g_object_set(G_OBJECT(g_video_lbls[label_id]), "text", label, NULL);
}

void VideoOut::set_font(const char *font)
{
    if (font == NULL || font[0] == '\0') return;
    for (int i = 0; i < MAX_LBL; ++i) {
        if (g_video_lbls[i] != NULL)
            g_object_set(G_OBJECT(g_video_lbls[i]), "font-desc", font, NULL);
    }
}
