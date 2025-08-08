#ifndef TELLO_APP_H
#define TELLO_APP_H

#include "tello.h"
#include "video_out.h"
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "socket_wrapper.h"

class TelloApp {
public:
    TelloApp();
    int run(int argc, char *argv[]);

private:
    static TelloApp *instance;

    enum ButtonId {
        CAMERA_VIDEO_BUTTON,
        MEDIA_FLOPPY_BUTTON,
        VIDEO_DISPLAY_BUTTON,
        SPEEDOMETER_BUTTON,
        PAN_UP_BUTTON,
        INPUT_GAMING_BUTTON,
        MAX_BUTTONS
    };

    Tello tello;
    VideoOut video_out;
    GtkWidget *buttons[MAX_BUTTONS];
    GtkWidget *infolabel;
    pid_t pid1;
    pid_t pid2;
    int button_amount;
    int input_file;
    Socket camera_socket1;
    Socket camera_socket2;
    struct sockaddr_in camera_address1;
    struct sockaddr_in camera_address2;

    int sky;
    int battery_percentage;
    int wifi_strength;
    int height;
    int fly_speed;
    int fly_time;
    float targetPosition_x;
    float targetPosition_y;
    float targetPosition_z;
    float targetDirection[2];
    float targetDistance;

    void input_key(int code, int value);
    void input_abs(int code, int value);
    void input_thread();
    static void input_thread_entry(TelloApp *self);
    int input_has(int fd, uint16_t type, uint16_t code);
    void close_input();
    int open_input();

    static gboolean key_callback_static(GtkWidget *widget, GdkEventKey *event, gpointer ptr);
    gboolean key_callback(GtkWidget *widget, GdkEventKey *event);

    static void button_callback_static(GtkWidget *widget, gpointer ptr);
    void button_callback(GtkWidget *widget, long id);
    void toggle_button(GtkWidget *widget, int value);

    static gboolean update_gui_static(gpointer ptr);
    gboolean update_gui(long id);

    static void tello_data_callback_static(int id);
    void tello_data_callback(int id);
    static void tello_camera_callback_static(uint8_t *data, int size);
    void tello_camera_callback(uint8_t *data, int size);

    void connection_thread();
    static void connection_thread_entry(TelloApp *self);

    void create_button(const char *icon, GtkWidget *container, int toggle);
    static void on_activate_static(GtkApplication *app, gpointer user_data);
    void on_activate(GtkApplication *app);
};

#endif // TELLO_APP_H
