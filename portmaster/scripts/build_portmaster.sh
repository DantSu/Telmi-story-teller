#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
OUT_DIR="$ROOT_DIR/dist/portmaster-build"

mkdir -p "$OUT_DIR"

make -C "$ROOT_DIR/src/storyTeller" clean
make -C "$ROOT_DIR/src/storyTeller" \
  BUILD_DIR="$OUT_DIR" \
  PLATFORM=portmaster \
  CROSS_COMPILE=${CROSS_COMPILE:-aarch64-linux-gnu-}

mv -f "$OUT_DIR/storyTeller" "$OUT_DIR/telmi_rk3326.aarch64"
chmod +x "$OUT_DIR/telmi_rk3326.aarch64"

mkdir -p "$OUT_DIR/data"
mkdir -p "$OUT_DIR/libs"

echo "Built: $OUT_DIR/telmi_rk3326.aarch64"
