#include "tello_app.h"
#include <fcntl.h>
#include <linux/input.h>
#include <math.h>
#include <thread>
#include <gdk/gdkx.h>
#include <gst/video/videooverlay.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>

TelloApp *TelloApp::instance = nullptr;

TelloApp::TelloApp()
    : infolabel(nullptr), pid1(0), pid2(0), button_amount(0), input_file(0),
      sky(0), battery_percentage(0), wifi_strength(0), height(0),
      fly_speed(0), fly_time(0), targetPosition_x(0), targetPosition_y(0),
      targetPosition_z(0), targetDistance(0)
{
    for (int i = 0; i < MAX_BUTTONS; ++i)
        buttons[i] = nullptr;
    targetDirection[0] = 0;
    targetDirection[1] = 1;
}

void TelloApp::input_key(int code, int value)
{
    if (value != 1) return;
    switch (code) {
    case BTN_START:
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(buttons[4]),
            !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(buttons[4])));
        return;
    case BTN_SELECT:
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(buttons[1]),
            !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(buttons[1])));
        return;
    case BTN_A:
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(buttons[3]),
            !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(buttons[3])));
        return;
    case BTN_B:
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(buttons[2]),
            !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(buttons[2])));
        return;
    case BTN_X:
        targetPosition_x = tello.position_x;
        targetPosition_y = tello.position_y;
        targetPosition_z = tello.position_z;
        return;
    case BTN_Y:
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(buttons[5]),
            !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(buttons[5])));
        return;
    }
}

void TelloApp::input_abs(int code, int value)
{
    float v = (value + 0.5) / 32767.5;
    if (v > -0.2 && v < 0.2) v = 0;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(buttons[5])) == TRUE) {
        if (v == 0) return;
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(buttons[5]), FALSE);
    }
    switch (code) {
        case ABS_X: tello.left_x = v; break;
        case ABS_Y: tello.left_y = -v; break;
        case ABS_RX: tello.right_x = v; break;
        case ABS_RY: tello.right_y = -v; break;
    }
}

void TelloApp::input_thread()
{
    struct input_event event[8];
    while (input_file > 0) {
        int count = read(input_file, &event, sizeof(event)) / sizeof(struct input_event);
        for (int i = 0; i < count; i++) {
            switch (event[i].type) {
            case EV_KEY: input_key(event[i].code, event[i].value); break;
            case EV_ABS: input_abs(event[i].code, event[i].value); break;
            }
        }
    }
}

void TelloApp::input_thread_entry(TelloApp *self)
{
    self->input_thread();
}

int TelloApp::input_has(int fd, uint16_t type, uint16_t code)
{
    size_t nchar = KEY_MAX/8 + 1;
    unsigned char bits[nchar];
    ioctl(fd, EVIOCGBIT(type, sizeof(bits)), &bits);
    return bits[code/8] & (1 << (code % 8));
}

void TelloApp::close_input()
{
    if (input_file == 0) return;
    close(input_file);
    input_file = 0;
}

int TelloApp::open_input()
{
    close_input();
    char path[32];
    for (int i = 0; i < 32; i++) {
        sprintf(path, "/dev/input/event%d", i);
        if (access(path, F_OK) < 0) return -1;
        int file = open(path, O_RDONLY);
        if (file <= 0) continue;
        if (input_has(file, EV_ABS, ABS_X) && input_has(file, EV_ABS, ABS_Y) &&
            input_has(file, EV_ABS, ABS_RX) && input_has(file, EV_ABS, ABS_RY)) {
            char name[256];
            ioctl(file, EVIOCGNAME(sizeof(name)), name);
            printf("Input: %s\n", name);
            input_file = file;
            std::thread(&TelloApp::input_thread_entry, this).detach();
            return 0;
        }
        close(file);
    }
    return -1;
}

gboolean TelloApp::key_callback(GtkWidget *widget, GdkEventKey *event)
{
    switch (event->keyval) {
    case GDK_KEY_space:
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(buttons[4]),
            !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(buttons[4])));
        return TRUE;
    case GDK_KEY_Return:
        open_input();
        return TRUE;
    }
    return FALSE;
}

gboolean TelloApp::key_callback_static(GtkWidget *widget, GdkEventKey *event, gpointer ptr)
{
    TelloApp *self = static_cast<TelloApp*>(ptr);
    return self->key_callback(widget, event);
}

void TelloApp::button_callback(GtkWidget *widget, long id)
{
    switch (id) {
    case 0:
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) == FALSE) {
            if (pid1 <= 0) return;
            kill(pid1, SIGTERM);
            pid1 = 0;
        } else {
            if (pid1 > 0) return;
            pid1 = fork();
            if(pid1 == 0) {
                exit(EXIT_SUCCESS);
            }
        }
        return;
    case 1:
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) == FALSE) {
            if (pid2 <= 0) return;
            kill(pid2, SIGTERM);
            pid2 = 0;
        } else {
            if (pid2 > 0) return;
            pid2 = fork();
            if(pid2 == 0) {
                char *homedir = getenv("HOME");
                struct timespec ts;
                clock_gettime(CLOCK_REALTIME, &ts);
                char path[64];
                sprintf(path, "%s/tello%ld.mp4", homedir, ts.tv_sec);
                execlp("ffmpeg", "ffmpeg", "-use_wallclock_as_timestamps", "1", "-framerate", "30", "-i", "udp://0.0.0.0:11112", "-c", "copy", path, NULL);
                exit(EXIT_SUCCESS);
            }
        }
        return;
    case 2:
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) == FALSE)
            tello.camera_mode(0);
        else
            tello.camera_mode(1);
        return;
    case 3:
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) == FALSE)
            tello.speed_mode = 0;
        else
            tello.speed_mode = 1;
        return;
    case 4:
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) == FALSE)
            tello.land();
        else
            tello.takeoff();
        return;
    }
}

void TelloApp::button_callback_static(GtkWidget *widget, gpointer ptr)
{
    if (instance) instance->button_callback(widget, (long)ptr);
}

void TelloApp::toggle_button(GtkWidget *widget, int value)
{
    g_signal_handlers_block_by_func(widget, (gpointer)TelloApp::button_callback_static, NULL);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), value);
    g_signal_handlers_unblock_by_func(widget, (gpointer)TelloApp::button_callback_static, NULL);
}

gboolean TelloApp::update_gui(long id)
{
    char label[64];
    switch (id) {
    case 26:
        if(wifi_strength != tello.wifi_strength) {
            wifi_strength = tello.wifi_strength;
            sprintf(label, "%d%%", wifi_strength);
            video_out.set_label(WIFI_LBL, label);
        }
        break;
    case 86:
        if (sky != tello.sky) {
            sky = tello.sky;
            toggle_button(buttons[4], sky);
        }
        if(battery_percentage != tello.battery_percentage) {
            battery_percentage = tello.battery_percentage;
            sprintf(label, "%d%%", battery_percentage);
            video_out.set_label(BAT_LBL, label);
        }
        if(height != tello.height) {
            height = tello.height;
            sprintf(label, "%.1fm", height/10.0);
            video_out.set_label(ALT_LBL, label);
        }
        int new_fly_speed = sqrt(tello.north_speed*tello.north_speed + tello.east_speed*tello.east_speed);
        if(fly_speed != new_fly_speed || fly_time != tello.fly_time) {
            fly_speed = new_fly_speed;
            fly_time = tello.fly_time;
            sprintf(label, "%.1fm/s  %.2d:%.2d", fly_speed/10.0, fly_time/600, (fly_time/10)%60);
            gtk_label_set_text(GTK_LABEL(infolabel), label);
        }
        break;
    }
    return FALSE;
}

gboolean TelloApp::update_gui_static(gpointer ptr)
{
    if(instance) return instance->update_gui((long)ptr);
    return FALSE;
}

void TelloApp::tello_data_callback(int id)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(buttons[5])) == TRUE) {
        if (id == 29) {
            targetDirection[0] = targetPosition_x - tello.position_x;
            targetDirection[1] = targetPosition_z - tello.position_z;
            targetDistance = hypot(targetDirection[0], targetDirection[1]);
            float l = 1 / sqrt(targetDirection[0] * targetDirection[0] + targetDirection[1] * targetDirection[1]);
            targetDirection[0] *= l; targetDirection[1] *= l;
        }
        if (id == 2048) {
            tello.left_y = 0;
            tello.right_x = 0;
            if (targetDistance > 2) {
                float q[] = {tello.rotation_x, tello.rotation_y, tello.rotation_z, tello.rotation_w};
                float v[] = {2 * (q[0]*q[2] + q[3]*q[1]), 1 - 2 * (q[0]*q[0] + q[1]*q[1])};
                float l = 1 / sqrt(v[0] * v[0] + v[1] * v[1]);
                v[0] *= l; v[1] *= l;
                float a = atan2(v[0]*targetDirection[1]-v[1]*targetDirection[0], v[0]*targetDirection[0]+v[1]*targetDirection[1]);
                if (a < -1) a = -1; else if (a > 1) a = 1;
                tello.left_x = -a;
                if (a > -0.2 && a < 0.2) tello.right_y = 0.4; else tello.right_y = 0;
            } else {
                tello.left_x = 0;
                tello.right_y = 0;
            }
        }
    }
    g_idle_add(TelloApp::update_gui_static, (void*)(long)id);
}

void TelloApp::tello_data_callback_static(int id)
{
    if(instance) instance->tello_data_callback(id);
}

void TelloApp::tello_camera_callback(uint8_t *data, int size)
{
    if ((data[0] % 32) == 0 && data[1] == 0)
        tello.request_iframe();
    sendto(camera_socket1, &data[2], size-2, 0, (struct sockaddr *)&camera_address1, sizeof(camera_address1));
    sendto(camera_socket2, &data[2], size-2, 0, (struct sockaddr *)&camera_address2, sizeof(camera_address2));
}

void TelloApp::tello_camera_callback_static(uint8_t *data, int size)
{
    if(instance) instance->tello_camera_callback(data, size);
}

void TelloApp::connection_thread()
{
    open_input();

    while (tello.connect(6038, 2) < 0) printf("Connection Failed\n");
    printf("Connected\n");

    camera_socket1.reset(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    camera_socket2.reset(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));

    memset(&camera_address1, 0, sizeof(camera_address1));
    camera_address1.sin_family = AF_INET;
    camera_address1.sin_port = htons(11111);
    inet_pton(AF_INET, "0.0.0.0", &(camera_address1.sin_addr));

    memset(&camera_address2, 0, sizeof(camera_address2));
    camera_address2.sin_family = AF_INET;
    camera_address2.sin_port = htons(11112);
    inet_pton(AF_INET, "0.0.0.0", &(camera_address2.sin_addr));

    tello.data_callback = &TelloApp::tello_data_callback_static;
    tello.camera_callback = &TelloApp::tello_camera_callback_static;
    video_out.start_video();
}

void TelloApp::connection_thread_entry(TelloApp *self)
{
    self->connection_thread();
}

void TelloApp::create_button(const char *icon, GtkWidget *container, int toggle)
{
    GtkWidget *button;
    if (toggle == 0) {
        button = gtk_button_new_from_icon_name(icon, GTK_ICON_SIZE_BUTTON);
        g_signal_connect(button, "clicked", G_CALLBACK(TelloApp::button_callback_static), (gpointer)(long)button_amount);
    } else {
        button = gtk_toggle_button_new();
        GtkWidget *image = gtk_image_new_from_icon_name(icon, GTK_ICON_SIZE_BUTTON);
        gtk_button_set_image(GTK_BUTTON(button), image);
        g_signal_connect(button, "toggled", G_CALLBACK(TelloApp::button_callback_static), (gpointer)(long)button_amount);
    }
    gtk_action_bar_pack_start(GTK_ACTION_BAR(container), button);
    buttons[button_amount] = button;
    button_amount++;
}

void TelloApp::on_activate(GtkApplication *app)
{
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Tello");
    gtk_window_set_icon_name (GTK_WINDOW(window), "gpsd-logo");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    g_signal_connect(window, "key_press_event", G_CALLBACK(TelloApp::key_callback_static), this);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *video_screen = gtk_drawing_area_new();
    gtk_widget_set_vexpand(video_screen, TRUE);
    gtk_widget_set_hexpand(video_screen, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), video_screen, TRUE, TRUE, 0);

    video_out.init_video_screen(video_screen);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_set_homogeneous(GTK_BOX(box), TRUE);
    gtk_widget_set_vexpand(box, FALSE);
    gtk_widget_set_hexpand(box, TRUE);
    gtk_widget_set_valign(box, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(vbox), box, FALSE, TRUE, 0);

    GtkWidget *actionbar = gtk_action_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), actionbar, FALSE, TRUE, 0);

    create_button("camera-video", actionbar, 1);
    create_button("media-floppy", actionbar, 1);
    create_button("video-display", actionbar, 1);
    create_button("speedometer", actionbar, 1);
    create_button("pan-up", actionbar, 1);
    create_button("input-gaming", actionbar, 1);

    infolabel = gtk_label_new("");
    gtk_action_bar_pack_end(GTK_ACTION_BAR(actionbar), infolabel);

    gtk_widget_show_all(window);

    std::thread(&TelloApp::connection_thread_entry, this).detach();
}

void TelloApp::on_activate_static(GtkApplication *app, gpointer user_data)
{
    TelloApp *self = static_cast<TelloApp*>(user_data);
    self->on_activate(app);
}

int TelloApp::run(int argc, char *argv[])
{
    instance = this;
    tello.camera_callback = NULL;
    tello.data_callback = NULL;
    GtkApplication *app = gtk_application_new("com.tello.GtkApplication", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(TelloApp::on_activate_static), this);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    tello.disconnect();
    close_input();
    return status;
}
