#!/usr/bin/env bash
set -euo pipefail

readonly runtime_version="24.08"
readonly flathub_repo="https://dl.flathub.org/repo/flathub.flatpakrepo"

command -v flatpak >/dev/null
command -v flatpak-builder >/dev/null

# Keep the large runtime installation in the named devcontainer volume.
mkdir -p "${FLATPAK_USER_DIR:?FLATPAK_USER_DIR is not set}"

flatpak remote-add --user --if-not-exists flathub "${flathub_repo}"
flatpak install --user --noninteractive -y flathub \
	"org.freedesktop.Platform//${runtime_version}" \
	"org.freedesktop.Sdk//${runtime_version}"

echo "Flatpak Platform and SDK ${runtime_version} are ready."
echo "Run 'make' for an SDK-native app build or 'make flatpak' for the package."
