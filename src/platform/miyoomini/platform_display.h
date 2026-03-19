#ifndef PLATFORM_DISPLAY_H__
#define PLATFORM_DISPLAY_H__

#include "SDL2/SDL.h"

// Use RGB565 for Miyoo Mini (optimized for this device's display)
#define PLATFORM_SDL_PIXEL_FORMAT SDL_PIXELFORMAT_RGB565

#endif // PLATFORM_DISPLAY_H__
