PREFIX ?= /usr/local
DESTDIR ?=
MODE ?= 755
CXX ?= g++
PKG_CONFIG ?= pkg-config

APP_ID = io.github.mrzinger.TelloDeck
PKGS = gtk+-3.0 gdk-x11-3.0 gstreamer-1.0 gstreamer-video-1.0
SRCDIR := src
BUILDDIR := build
TARGET := tello

SOURCES := \
	$(SRCDIR)/main.cpp \
	$(SRCDIR)/tello.cpp \
	$(SRCDIR)/video_out.cpp \
	$(SRCDIR)/crc_utils.cpp \
	$(SRCDIR)/tello_app.cpp

OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

CPPFLAGS += -I$(SRCDIR) $(shell $(PKG_CONFIG) --cflags $(PKGS))
CXXFLAGS ?= -O2
CXXFLAGS += -Wall -pthread
LDLIBS += -lm -pthread $(shell $(PKG_CONFIG) --libs $(PKGS))

all: $(TARGET)

# The devcontainer itself is the Freedesktop SDK, so this target builds against
# the same headers and libraries used by the Flatpak manifest.
sdk: all

flatpak:
	flatpak-builder --user --force-clean --disable-rofiles-fuse \
		--install-deps-from=flathub \
		build-dir io.github.mrzinger.TelloDeck.yml

flatpak-install:
	flatpak-builder --user --force-clean --disable-rofiles-fuse --install \
		--install-deps-from=flathub \
		build-dir io.github.mrzinger.TelloDeck.yml

flatpak-run:
	flatpak run $(APP_ID)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILDDIR):
	mkdir -p $@

debug: CXXFLAGS := -O0 -g -Wall -pthread
debug: clean $(TARGET)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

-include $(DEPS)

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

.PHONY: all sdk flatpak flatpak-install flatpak-run clean install uninstall
