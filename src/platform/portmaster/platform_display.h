#ifndef PLATFORM_DISPLAY_H__
#define PLATFORM_DISPLAY_H__

#include "SDL2/SDL.h"

// Use ARGB8888 for PortMaster devices (R36S, RK3326-based handhelds)
// This prevents color inversion issues on these devices
#define PLATFORM_SDL_PIXEL_FORMAT SDL_PIXELFORMAT_ARGB8888

#endif // PLATFORM_DISPLAY_H__
