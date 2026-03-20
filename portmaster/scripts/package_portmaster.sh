#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="$ROOT_DIR/dist/portmaster-build"
PORTS_DIR="$ROOT_DIR/dist/portmaster"
APP_DIR="$PORTS_DIR/telmi"
PORTMASTER_BINARY="telmi_rk3326.aarch64"
DEVICE_ARCH="${DEVICE_ARCH:-aarch64}"
PORT_SCRIPT_NAME="Telmi.sh"

if [[ ! -f "$BUILD_DIR/$PORTMASTER_BINARY" ]]; then
  echo "Missing binary: $BUILD_DIR/$PORTMASTER_BINARY"
  echo "Run ./portmaster/scripts/build_portmaster.sh first."
  exit 1
fi

rm -rf "$PORTS_DIR"
mkdir -p "$APP_DIR"

cp "$ROOT_DIR/portmaster/telmi.sh" "$PORTS_DIR/$PORT_SCRIPT_NAME"
chmod +x "$PORTS_DIR/$PORT_SCRIPT_NAME"

cp "$ROOT_DIR/portmaster/telmi.gptk" "$APP_DIR/telmi.gptk"
cp "$BUILD_DIR/$PORTMASTER_BINARY" "$APP_DIR/$PORTMASTER_BINARY"
chmod +x "$APP_DIR/$PORTMASTER_BINARY"

mkdir -p "$APP_DIR/data/res" "$APP_DIR/data/Music" "$APP_DIR/data/Stories" "$APP_DIR/data/Saves/Stories" "$APP_DIR/libs.${DEVICE_ARCH}"

if [[ -d "$ROOT_DIR/src/storyTeller/res" ]]; then
  cp -R "$ROOT_DIR"/src/storyTeller/res/. "$APP_DIR/data/res/"
fi

if [[ -d "$ROOT_DIR/static/build/.tmp_update/res" ]]; then
  cp -R "$ROOT_DIR"/static/build/.tmp_update/res/. "$APP_DIR/data/res/"
fi

if [[ ! -f "$APP_DIR/data/res/Exo2-Regular.ttf" || ! -f "$APP_DIR/data/res/Exo2-Bold.ttf" ]]; then
  echo "Missing required fonts in package: Exo2-Regular.ttf / Exo2-Bold.ttf"
  exit 1
fi

if [[ ! -f "$APP_DIR/data/res/selectStories.png" ]]; then
  echo "Missing required UI resources in package (expected selectStories.png)"
  exit 1
fi

echo "PortMaster package ready: $PORTS_DIR"
echo "Copy contents of dist/portmaster/ to /roms/ports/ on your SD card"
