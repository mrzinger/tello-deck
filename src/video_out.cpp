#include "video_out.h"
#include <stdlib.h>

const char *VideoOut::g_def_label[MAX_LBL] = {"📶Wifi:", "↑Alt:", "🔋Bat:"};

VideoOut::VideoOut() : pipeline(NULL) {
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

GtkWidget *VideoOut::init_video_screen()
{
    gst_init(NULL, NULL);

    GstElement *udpsrc = gst_element_factory_make("udpsrc", "udpsrc");
    GstElement *decodebin = gst_element_factory_make("decodebin", "decodebin");
    GstElement *queue = gst_element_factory_make("queue", "queue");
    GstElement *videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    GstElement *gtk_sink = gst_element_factory_make("gtkglsink", "gtkglsink");
    GstElement *sink = gst_element_factory_make("glsinkbin", "glsinkbin");
    GstElement *wifi_label = gst_element_factory_make("textoverlay", "textoverlay1");
    GstElement *alt_label = gst_element_factory_make("textoverlay", "textoverlay2");
    GstElement *bat_label = gst_element_factory_make("textoverlay", "textoverlay3");

    if (!udpsrc || !decodebin || !queue || !videoconvert || !gtk_sink || !sink ||
        !wifi_label || !alt_label || !bat_label) {
        g_warning("Could not create the GStreamer video elements");
        return gtk_label_new("Video output is unavailable");
    }

    GtkWidget *video_widget = NULL;
    g_object_set(G_OBJECT(sink), "sink", gtk_sink, NULL);
    g_object_get(G_OBJECT(gtk_sink), "widget", &video_widget, NULL);
    if (video_widget == NULL) {
        g_warning("Could not create the GTK video widget");
        return gtk_label_new("Video output is unavailable");
    }

    g_object_set(G_OBJECT(udpsrc), "uri", "udp://0.0.0.0:11111", NULL);
    g_object_set(G_OBJECT(queue), "max-size-buffers", 1, "max-size-time", 0,
                 "max-size-bytes", 0, "leaky", 2, NULL);
    g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);

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
        return video_widget;
    }

    if (!gst_element_link_many(queue, videoconvert, wifi_label, alt_label, bat_label, sink, NULL))
    {
        g_printerr("Failed to link the video processing pipeline.\n");
        return video_widget;
    }

    g_signal_connect(decodebin, "pad-added", G_CALLBACK(VideoOut::gst_pad_link_elements), queue);
    return video_widget;
}

void VideoOut::set_label(VideoLabels label_id, const char *text)
{
    if (g_video_lbls[label_id] == NULL) return;
    char label[64];
    sprintf(label, "%s %s", g_def_label[label_id], text);
    g_object_set(G_OBJECT(g_video_lbls[label_id]), "text", label, NULL);
}
