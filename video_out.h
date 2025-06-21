#ifndef _VIDEO_OUT_H_
#define _VIDEO_OUT_H_

#include <gst/gst.h>
#include <gdk/gdkx.h>
#include <gst/video/videooverlay.h>
#include <gtk/gtk.h>

typedef enum {
    WIFI_LBL = 0, //Wifi label
    ALT_LBL, //Altitude label
    BAT_LBL, //Battery label
    MAX_LBL
} VideoLabels;

class VideoOut {
public:
    VideoOut();

    void init_video_screen(GtkWidget *video_screen_parent);
    void set_label(VideoLabels label_id, const char *text);
    void start_video();

private:
    GstElement *pipeline;
    GstElement *g_video_lbls[MAX_LBL];
    guintptr video_window_handle;
    static const char *g_def_label[MAX_LBL];

    static GstBusSyncReply bus_sync_handler(GstBus *bus, GstMessage *message, gpointer user_data);
    static void realize_cb(GtkWidget *widget, gpointer data);
    static gboolean delete_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data);
    static void gst_pad_link_elements(GstElement *element, GstPad *pad, gpointer data);
};

#endif /* _VIDEO_OUT_H_ */
