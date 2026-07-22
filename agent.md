# Agent Guide

## Project overview

Tello Deck is a Linux C++ desktop application that turns a Steam Deck or other
SDL-compatible controller into a controller for a DJI Tello drone. It uses GTK 3
for the interface, GStreamer for the live H.264 video stream, SDL2 for controller
input, UDP sockets for the drone protocol, and the `ffmpeg` executable for MP4
recording.

## Repository map

- `src/main.cpp` starts the application.
- `src/tello_app.{h,cpp}` owns the GTK application, controller input, UI state,
  recording process, and coordination between the drone and video components.
- `src/tello.{h,cpp}` implements the Tello UDP protocol, telemetry parsing, and
  background network threads.
- `src/video_out.{h,cpp}` constructs the GStreamer pipeline and telemetry
  overlays, with separate X11 and Wayland-compatible sink paths.
- `src/socket_wrapper.h` provides move-only RAII ownership for socket file
  descriptors.
- `src/tello_cmd.h` contains protocol command identifiers.
- `src/crc_utils.{h,cpp}` contains packet checksum helpers.
- `makefile` is the canonical native build definition.
- `io.github.mrzinger.TelloDeck.yml` is the Flatpak manifest.
- `data/` contains desktop integration and AppStream assets.
- `.github/workflows/` defines the native and Flatpak CI builds.

## Build and validation

Use commands from the repository root.

```sh
make
make debug
make clean
```

`make` is the same native build exercised by CI. `make debug` performs a clean
build with `-O0 -g -Wall -pthread`. The required pkg-config packages are
`gtk+-3.0`, `gdk-x11-3.0`, `gstreamer-1.0`, `gstreamer-video-1.0`, and `sdl2`.

For packaging changes, validate the Flatpak manifest as well:

```sh
make flatpak
```

This may download runtimes and dependencies. `make flatpak-install` additionally
installs the result for the current user, so do not use it unless installation is
part of the task.

There is currently no automated test suite. For every code change, at minimum:

1. Run `make` (or `make debug` when investigating runtime behavior).
2. Check compiler warnings and do not introduce new ones.
3. For Flatpak, dependency, desktop metadata, or display-backend changes, also
   build the Flatpak when the necessary tooling is available.
4. State clearly which runtime paths were not exercised, especially real drone,
   controller, X11/Wayland, video, and recording behavior.

Do not treat a successful launch without a drone as validation of networking or
flight controls.

## Coding guidelines

- Follow the style of the file being edited. Avoid unrelated formatting
  churn.
- Keep declarations in the matching header and implementation in the `.cpp`
  file. Prefer small, targeted changes over moving responsibilities between
  components without a clear need.
- Preserve RAII ownership for sockets and GStreamer objects. Check allocation,
  element creation, socket, `fork`, and process-launch failures where relevant.
- GTK widgets must be created and updated on the GTK main thread. Marshal work
  from detached networking threads through GLib/GTK callbacks rather than
  touching widgets directly.
- Treat protocol packet layout, byte order, CRC coverage, command IDs, UDP ports,
  axis packing, and controller dead zones as compatibility-sensitive behavior.
  Explain and verify any changes to them carefully.
- Keep the video pipeline valid on both native X11 and Wayland/Flatpak paths.
  Changes to sinks, overlays, or caps should account for both paths.
- Avoid adding dependencies unless they are reflected in the native build, CI,
  and Flatpak manifest as applicable.
- Keep user-visible application IDs and filenames consistent with
  `io.github.mrzinger.TelloDeck` across the manifest and `data/` metadata.

## Hardware and process safety

- Never connect to or send commands to a real drone during routine validation.
  Live-hardware testing requires an explicit request from the user and a human
  operator who has confirmed a safe flight area.
- Treat takeoff, landing, stick-control, and return-mode changes as
  safety-critical. Preserve neutral-input and disconnect behavior.
- Recording starts a child `ffmpeg` process. Ensure changes clean up child
  processes and do not overwrite existing recordings.
- Do not broaden Flatpak permissions without explaining why the application
  needs them. Current permissions cover networking, display access, controllers,
  and the user's Videos directory.

## Change discipline

- Inspect `git status` before editing and preserve unrelated user changes.
- Do not commit generated outputs such as `tello`, `build/`, `build-dir/`,
  `.flatpak-builder/`, Flatpak repositories, or `.flatpak` bundles.
- Update `README.md`, desktop metadata, or the Flatpak manifest when a change
  affects installation, controls, permissions, runtime dependencies, or
  user-visible behavior.
- In the final report, summarize changed files and validation performed, and call
  out any hardware- or environment-dependent checks that remain.
- Update agent.md in case if any of the conditions described above change. Inform the user about the change.
