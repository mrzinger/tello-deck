#include "input_controller.h"
#include "../tello.h"
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>

InputController::InputController(Tello& t, std::function<void(int,int)> button_cb)
    : tello(t), input_file(0), button_callback(button_cb) {}

InputController::~InputController() {
    close();
}

int InputController::input_has(int fd, uint16_t type, uint16_t code) {
    size_t nchar = KEY_MAX/8 + 1;
    unsigned char bits[nchar];
    ioctl(fd, EVIOCGBIT(type, sizeof(bits)), &bits);
    return bits[code/8] & (1 << (code % 8));
}

void InputController::handle_abs(int code, int value) {
    float v = (value + 0.5f) / 32767.5f;
    if (v > -0.2f && v < 0.2f) v = 0;
    switch (code) {
    case ABS_X: tello.left_x = v; break;
    case ABS_Y: tello.left_y = -v; break;
    case ABS_RX: tello.right_x = v; break;
    case ABS_RY: tello.right_y = -v; break;
    }
}

void InputController::handle_key(int code, int value) {
    if (button_callback) button_callback(code, value);
}

void InputController::thread_func() {
    struct input_event event[8];
    while (input_file > 0) {
        int count = read(input_file, &event, sizeof(event)) / sizeof(struct input_event);
        for (int i = 0; i < count; i++) {
            switch (event[i].type) {
            case EV_KEY: handle_key(event[i].code, event[i].value); break;
            case EV_ABS: handle_abs(event[i].code, event[i].value); break;
            }
        }
    }
}

int InputController::open() {
    close();
    char path[32];
    for (int i = 0; i < 32; i++) {
        sprintf(path, "/dev/input/event%d", i);
        if (access(path, F_OK) < 0) return -1;
        int file = ::open(path, O_RDONLY);
        if (file <= 0) continue;
        if (input_has(file, EV_ABS, ABS_X) && input_has(file, EV_ABS, ABS_Y) &&
            input_has(file, EV_ABS, ABS_RX) && input_has(file, EV_ABS, ABS_RY)) {
            char name[256];
            ioctl(file, EVIOCGNAME(sizeof(name)), name);
            printf("Input: %s\n", name);
            input_file = file;
            thread = std::thread(&InputController::thread_func, this);
            thread.detach();
            return 0;
        }
        ::close(file);
    }
    return -1;
}

void InputController::close() {
    if (input_file == 0) return;
    ::close(input_file);
    input_file = 0;
}

