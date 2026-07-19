#ifndef _VIDEO_OUT_H_
#define _VIDEO_OUT_H_

#include <gst/gst.h>
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
    ~VideoOut();

    GtkWidget *init_video_screen();
    void set_label(VideoLabels label_id, const char *text);
    void start_video();

private:
    GstElement *pipeline;
    GstElement *g_video_lbls[MAX_LBL];
    static const char *g_def_label[MAX_LBL];

    static void gst_pad_link_elements(GstElement *element, GstPad *pad, gpointer data);
};

#endif /* _VIDEO_OUT_H_ */
