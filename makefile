PREFIX ?= /usr/local
DESTDIR ?=
MODE ?= 755
CXX ?= g++
PKG_CONFIG ?= pkg-config

APP_ID = io.github.mrzinger.TelloDeck
PKGS = gtk+-3.0 gdk-x11-3.0 gstreamer-1.0 gstreamer-video-1.0
SOURCES = main.cpp tello.cpp video_out.cpp crc_utils.cpp src/tello_app.cpp

CPPFLAGS += -I. -Isrc $(shell $(PKG_CONFIG) --cflags $(PKGS))
CXXFLAGS ?= -O2
CXXFLAGS += -Wall -pthread
LDLIBS += -lm -pthread $(shell $(PKG_CONFIG) --libs $(PKGS))

all: tello

tello: $(SOURCES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(SOURCES) -o $@ $(LDFLAGS) $(LDLIBS)

debug: CXXFLAGS := -O0 -g -Wall -pthread
debug: clean tello
clean:
	rm -f tello

install: all
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m $(MODE) tello $(DESTDIR)$(PREFIX)/bin/tello
	install -d $(DESTDIR)$(PREFIX)/share/applications
	install -m 644 data/$(APP_ID).desktop $(DESTDIR)$(PREFIX)/share/applications/
	install -d $(DESTDIR)$(PREFIX)/share/metainfo
	install -m 644 data/$(APP_ID).metainfo.xml $(DESTDIR)$(PREFIX)/share/metainfo/
	install -d $(DESTDIR)$(PREFIX)/share/icons/hicolor/scalable/apps
	install -m 644 data/$(APP_ID).svg $(DESTDIR)$(PREFIX)/share/icons/hicolor/scalable/apps/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/tello
	rm -f $(DESTDIR)$(PREFIX)/share/applications/$(APP_ID).desktop
	rm -f $(DESTDIR)$(PREFIX)/share/metainfo/$(APP_ID).metainfo.xml
	rm -f $(DESTDIR)$(PREFIX)/share/icons/hicolor/scalable/apps/$(APP_ID).svg

.PHONY: all clean install uninstall
