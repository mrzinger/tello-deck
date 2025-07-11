DESTDIR = /usr/local
MODE = 755
CC = g++
CFLAGS = -Wall `pkg-config --cflags gtk+-3.0 gstreamer-1.0 gstreamer-video-1.0`
LIBS = -lm -lpthread `pkg-config --libs gtk+-3.0 gstreamer-1.0 gstreamer-video-1.0 gdk-x11-3.0` 
CFLAGS += -I/usr/lib/steamos/modules/6.0.2-arch1-1.1/build/include/uapi/
CFLAGS += -I/usr/lib/steamos/modules/6.0.2-arch1-1.1/build/arch/x86/include/uapi/
CFLAGS += -I/usr/include/gstreamer-1.0/
CFLAGS += -I/usr/include/glib-2.0/
CFLAGS += -I/usr/include/gtk-3.0/
CFLAGS += -I/usr/lib/glib-2.0/include/
CFLAGS += -I/usr/include/pango-1.0/
CFLAGS += -I/usr/include/harfbuzz/
CFLAGS += -I/usr/include/cairo/
CFLAGS += -I/usr/include/gdk-pixbuf-2.0/
CFLAGS += -I/usr/include/libdrm/
CFLAGS += -I/usr/include/libusb-1.0/
CFLAGS += -I/usr/include/atk-1.0/


all: main.cpp tello.cpp
	$(CC) -O2 $(CFLAGS) main.cpp tello.cpp video_out.cpp crc_utils.cpp -o tello $(LIBS)

debug: main.cpp tello.cpp
	$(CC) -g $(CFLAGS) main.cpp tello.cpp video_out.cpp crc_utils.cpp -o tello $(LIBS)
clean:
	rm -f tello

install: all
	install -d ${DESTDIR}/bin
	install -m ${MODE} tello ${DESTDIR}/bin

uninstall:
	rm -f ${DESTDIR}/bin/tello

.PHONY: all clean install uninstall
