#!/bin/bash
XDG_DATA_HOME=${XDG_DATA_HOME:-$HOME/.local/share}

if [ -d "/opt/system/Tools/PortMaster" ]; then
  controlfolder="/opt/system/Tools/PortMaster"
elif [ -d "/opt/tools/PortMaster/" ]; then
  controlfolder="/opt/tools/PortMaster"
else
  controlfolder="/roms/ports/PortMaster"
fi
source "$controlfolder/control.txt"

if [ -f "$controlfolder/device_info.txt" ]; then
  source "$controlfolder/device_info.txt"
fi

if command -v get_controls >/dev/null 2>&1; then
  get_controls
fi

GAMEDIR="/roms/ports/telmi_app"
cd "$GAMEDIR" || exit 1

echo "[telmi] launcher invoked $(date)" >> "$GAMEDIR/log.txt" 2>/dev/null
exec > >(tee -a "$GAMEDIR/log.txt") 2>&1

export LD_LIBRARY_PATH="$GAMEDIR/libs:$LD_LIBRARY_PATH"
export GAMEDIR
export XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/tmp/runtime-ark}"
mkdir -p "$XDG_RUNTIME_DIR" 2>/dev/null
export SDL_AUDIODRIVER="${SDL_AUDIODRIVER:-alsa}"
export SDL_VIDEODRIVER="${SDL_VIDEODRIVER:-kmsdrm}"
export SDL_RENDER_DRIVER="${SDL_RENDER_DRIVER:-software}"
export SDL_KMSDRM_REQUIRE_DRM_MASTER="${SDL_KMSDRM_REQUIRE_DRM_MASTER:-0}"

echo "[telmi] GAMEDIR=$GAMEDIR"
echo "[telmi] Launching at $(date)"
echo "[telmi] SDL_VIDEODRIVER=$SDL_VIDEODRIVER SDL_RENDER_DRIVER=$SDL_RENDER_DRIVER"

BACKLIGHT_DIR="/sys/class/backlight/backlight"
if [ -r "$BACKLIGHT_DIR/max_brightness" ] && [ -r "$BACKLIGHT_DIR/brightness" ]; then
  MAX_BRIGHTNESS=$(cat "$BACKLIGHT_DIR/max_brightness" 2>/dev/null)
  CUR_BRIGHTNESS=$(cat "$BACKLIGHT_DIR/brightness" 2>/dev/null)
  if [ -n "$MAX_BRIGHTNESS" ] && [ "$MAX_BRIGHTNESS" -gt 0 ] && [ "$CUR_BRIGHTNESS" = "0" ]; then
    echo "[telmi] backlight was 0, restoring to max"
    if [ -w "$BACKLIGHT_DIR/brightness" ]; then
      echo "$MAX_BRIGHTNESS" > "$BACKLIGHT_DIR/brightness" 2>/dev/null || true
    else
      $ESUDO sh -c "echo $MAX_BRIGHTNESS > $BACKLIGHT_DIR/brightness" 2>/dev/null || true
    fi
    sleep 0.1
  fi
fi

if [ ! -x "$GAMEDIR/telmi_rk3326.aarch64" ]; then
  echo "[telmi] ERROR: binary missing or not executable: $GAMEDIR/telmi_rk3326.aarch64"
  exit 1
fi

if [ ! -f "$GAMEDIR/data/res/Exo2-Regular.ttf" ] || [ ! -f "$GAMEDIR/data/res/Exo2-Bold.ttf" ]; then
  echo "[telmi] ERROR: required fonts missing in $GAMEDIR/data/res"
  exit 1
fi

if [ ! -f "$GAMEDIR/data/res/selectStories.png" ]; then
  echo "[telmi] ERROR: required UI assets missing in $GAMEDIR/data/res"
  exit 1
fi

GPTOKEYB_CMD="${GPTOKEYB:-}"
if [ -z "$GPTOKEYB_CMD" ] && command -v gptokeyb >/dev/null 2>&1; then
  GPTOKEYB_CMD="$(command -v gptokeyb)"
fi
if [ -z "$GPTOKEYB_CMD" ] && [ -x "$controlfolder/gptokeyb" ]; then
  GPTOKEYB_CMD="$controlfolder/gptokeyb"
fi

if [ -n "$GPTOKEYB_CMD" ]; then
  "$GPTOKEYB_CMD" "$GAMEDIR/telmi_rk3326.aarch64" -c "$GAMEDIR/telmi.gptk" &
else
  echo "[telmi] WARNING: GPTOKEYB is not set; continuing without gamepad key mapping"
fi

"$GAMEDIR/telmi_rk3326.aarch64"
APP_EXIT_CODE=$?
echo "[telmi] App exit code: $APP_EXIT_CODE"

$ESUDO kill -9 $(pidof gptokeyb) 2>/dev/null

if [ "$APP_EXIT_CODE" = "194" ] || [ -f "/tmp/telmi_poweroff.flag" ]; then
  echo "[telmi] Poweroff requested by app"
  rm -f /tmp/telmi_poweroff.flag
  sync
  $ESUDO poweroff
  exit 0
fi

$ESUDO systemctl restart ui

exit $APP_EXIT_CODE
