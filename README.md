# Tello-Deck

Transform your Steam Deck into a controller for the DJI Tello drone.

This project incorporates:
* GTK-3 for the GUI.
* GStreamer framework and plug-ins for video and data streaming.
* ffmpeg for recording and saving video.

Special thanks to Suphi and his project (https://gitlab.com/Suphi/Tello) for providing the foundation for this app.

## Controls

- <kbd>☰</kbd>: Takeoff/Land
- <kbd>⧉</kbd>: Start/Stop Recording
- <kbd>Ⓐ</kbd>: Toggle Speed Mode
- <kbd>Ⓑ</kbd>: Camera Mode
- <kbd>Ⓧ</kbd>: Not Assigned
- <kbd>Ⓨ</kbd>: Scan for Gamepad
![](ScreenShot.jpg)

## Installation
```bash
make install
```
tello is installed into /usr/local by default this can be changed by setting DESTDIR.
```bash 
make DESTDIR=/usr install
```

### Removal
```bash
make uninstall
```

### Running in Gaming Mode
To run in Gaming Mode, navigate to /usr/local/bin/ (or installation directory) in Dolphin, right-click the tello executable, and select "Add to Steam".

## Building on SteamDeck
### Disable read-only mode
By default SteamDeck's root file system is read-only so after you create root credentials you have to disable it:
```bash
sudo steamos-readonly disable
```
or
```bash
sudo btrfs property set -ts / ro false
```
### Installing development tools
Install `gcc` and `pkg-config`:
```bash
sudo pacman -S gcc pkg-config
```

### Install dependencies and libraries
Install essential libraries and dependencies for development:
Install `linux-headers`
```bash
sudo pacman -S glibc linux-api-headers
```
Install `gstreamer` and the `plug-ins`
```bash
sudo pacman -S gstreamer gst-plugins-base gst-plugins-base-lib gst-plugins-good gst-libav
```
Install libs needed for overlays
```bash
sudo pacman -S cairo pango harfbuzz gdk-pixbuf2 
```
Install X Window related libraries
```bash
sudo pacman -S xorgproto libx11 at-spi2-core 
```
### For further development
To support network management capabilities and be able to automatically join Tello's Wi-fi network we will need `libnm` and `glib2`
```bash
sudo pacman -S libnm glib2
```