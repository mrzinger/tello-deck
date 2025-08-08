#pragma once

#include <functional>
#include <thread>
#include <cstdint>

class Tello;

class InputController {
public:
    InputController(Tello& tello, std::function<void(int,int)> button_cb = {});
    ~InputController();

    int open();
    void close();

private:
    int input_has(int fd, uint16_t type, uint16_t code);
    void handle_abs(int code, int value);
    void handle_key(int code, int value);
    void thread_func();

    Tello& tello;
    int input_file;
    std::thread thread;
    std::function<void(int,int)> button_callback;
};

