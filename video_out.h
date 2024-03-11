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

void init_video_screen (GtkWidget *video_screen_parent);
void set_label (VideoLabels label_id, const char *text);
void start_video();

#endif /* _VIDEO_OUT_H_ */