DESTDIR = /usr/local
MODE = 755
CC = gcc
CFLAGS = -Wall `pkg-config --cflags gtk+-3.0 gstreamer-1.0 gstreamer-video-1.0`
LIBS = -lm -lpthread `pkg-config --libs gtk+-3.0 gstreamer-1.0 gstreamer-video-1.0 gdk-x11-3.0` 
CFLAGS += -I/usr/lib/steamos/modules/6.0.2-arch1-1.1/build/include/uapi/
CFLAGS += -I/usr/lib/steamos/modules/6.0.2-arch1-1.1/build/arch/x86/include/uapi/
CFLAGS += -I/usr/include/gstreamer-1.0/

all: main.c tello.c
	$(CC) -O2 $(CFLAGS) $(LIBS) main.c tello.c video_out.c -o tello

debug: main.c tello.c
	$(CC) -g $(CFLAGS) $(LIBS) main.c tello.c video_out.c -o tello

clean:
	rm -f tello

install: all
	install -d ${DESTDIR}/bin
	install -m ${MODE} tello ${DESTDIR}/bin

uninstall:
	rm -f ${DESTDIR}/bin/tello

.PHONY: all clean install uninstall
