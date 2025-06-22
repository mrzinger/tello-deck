#ifndef _TELLO_H_
#define _TELLO_H_

#include <stddef.h>
#include <arpa/inet.h>
#include <stdint.h>

class Tello {
public:
    int data_socket;
    int camera_socket;
    struct sockaddr_in address;
    void (*data_callback)(int);
    void (*camera_callback)(uint8_t *, int);
    uint16_t sequence;
    uint8_t speed_mode;
    float left_x;
    float left_y;
    float right_x;
    float right_y;
    uint16_t battery_left;
    uint8_t battery_low;
    uint8_t battery_lower;
    uint8_t battery_percentage;
    uint8_t battery_state;
    uint8_t camera_state;
    uint8_t visual_state;
    uint8_t hover;
    uint8_t open;
    uint8_t sky;
    uint8_t ground;
    int16_t east_speed;
    uint8_t electrical_machinery_state;
    uint8_t factory_mode;
    uint8_t fly_mode;
    uint16_t fly_time;
    uint16_t fly_time_left;
    uint8_t front_in;
    uint8_t front_lsc;
    uint8_t front_out;
    uint8_t gravity_state;
    uint16_t ground_speed;
    int16_t height;
    uint8_t imu_calibration_state;
    uint8_t imu_state;
    uint8_t light_strength;
    int16_t north_speed;
    uint8_t outage_recording;
    uint8_t power_state;
    uint8_t pressure_state;
    uint8_t temperature_height;
    uint8_t throw_fly_timer;
    uint8_t wifi_disturb;
    uint8_t wifi_strength;
    uint8_t wind_state;
    int16_t velocity_x;
    int16_t velocity_y;
    int16_t velocity_z;
    float position_x;
    float position_y;
    float position_z;
    float position_uncertainty;
    float rotation_x;
    float rotation_y;
    float rotation_z;
    float rotation_w;
    float relative_velocity_x;
    float relative_velocity_y;
    float relative_velocity_z;

    Tello() : data_socket(0), camera_socket(0), data_callback(nullptr),
              camera_callback(nullptr), sequence(0), speed_mode(0),
              left_x(0), left_y(0), right_x(0), right_y(0) {}

    int connect(int camera_port, int timeout);
    void disconnect();
    void request_iframe();
    void camera_mode(int value);
    void takeoff();
    void land();

private:
    static void *poll_thread(void *ptr);
    static void *data_thread(void *ptr);
    static void *camera_thread(void *ptr);
    static int crc_8(uint8_t message[], int length);
    static int crc_16(uint8_t message[], int length);
    void send_package(int type, int id, int sequence, int *data, int datasize);
};

#endif
