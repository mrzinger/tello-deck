#ifndef SOCKET_WRAPPER_H
#define SOCKET_WRAPPER_H
#include <unistd.h>

class Socket {
    int fd;
public:
    Socket(int fd_ = -1) : fd(fd_) {}
    ~Socket() { close(); }
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept : fd(other.fd) { other.fd = -1; }
    Socket& operator=(Socket&& other) noexcept {
        if (this != &other) {
            close();
            fd = other.fd;
            other.fd = -1;
        }
        return *this;
    }
    int get() const { return fd; }
    operator int() const { return fd; }
    void reset(int newfd = -1) {
        if (fd != -1) ::close(fd);
        fd = newfd;
    }
    void close() {
        if (fd != -1) {
            ::close(fd);
            fd = -1;
        }
    }
};

#endif // SOCKET_WRAPPER_H
