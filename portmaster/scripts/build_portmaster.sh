#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
OUT_DIR="$ROOT_DIR/dist/portmaster-build"
PORTMASTER_BINARY="telmi_rk3326.aarch64"
DEVICE_ARCH="${DEVICE_ARCH:-aarch64}"

mkdir -p "$OUT_DIR"

make -C "$ROOT_DIR/src/storyTeller" clean
make -C "$ROOT_DIR/src/storyTeller" \
  BUILD_DIR="$OUT_DIR" \
  PLATFORM=portmaster \
  CROSS_COMPILE=${CROSS_COMPILE:-aarch64-linux-gnu-}

mv -f "$OUT_DIR/storyTeller" "$OUT_DIR/$PORTMASTER_BINARY"
chmod +x "$OUT_DIR/$PORTMASTER_BINARY"

mkdir -p "$OUT_DIR/data"
mkdir -p "$OUT_DIR/libs.${DEVICE_ARCH}"

echo "Built: $OUT_DIR/$PORTMASTER_BINARY"
