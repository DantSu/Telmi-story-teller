#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include "system/battery.h"
#include "system/device_model.h"
#include "system/display.h"
#include "system/keymap_hw.h"
#include "system/rumble.h"
#include "system/system.h"
#include "utils/file.h"
#include "utils/log.h"
#include "utils/msleep.h"

#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"

#define RELEASED 0
#define PRESSED 1
#define REPEATING 2

#define DISPLAY_TIMEOUT 10000

static bool quit = false;
static bool suspended = false;
static int input_fd;
static struct input_event ev;
static struct pollfd fds[1];

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Surface *screen = NULL;
SDL_Texture *texture = NULL;

void applyScreen(void) {
    SDL_RenderClear(renderer);
    SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void suspend(bool enabled) {
    suspended = enabled;
    if (suspended) {
        SDL_FillRect(screen, NULL, 0);
        applyScreen();
    }
    // system_powersave(suspended);
    display_setScreen(!suspended);
}

static void sigHandler(int sig) {
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            quit = true;
            break;
        default:
            break;
    }
}

int main(void) {
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    bool turn_off = false;

    getDeviceModel();

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    window = SDL_CreateWindow("main", 0, 0, 640, 480, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    screen = SDL_CreateRGBSurface(0, 640, 480, 32, 0, 0, 0, 0);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, screen->w, screen->h);

    int min_delay = 15;
    int frame_delay = 80;
    int frame_count = 0;

    SDL_Surface *frames[24];
    SDL_Surface *image;

    for (int i = 0; i < 24; i++) {
        char image_path[STR_MAX + 50];
        snprintf(image_path, STR_MAX + 49, "%schargingState%d.png", SYSTEM_RESOURCES, i);
        if ((image = IMG_Load(image_path)))
            frames[frame_count++] = image;
    }

    char json_path[STR_MAX + 20];
    snprintf(json_path, STR_MAX + 19, "%schargingState.json", SYSTEM_RESOURCES);
    if (is_file(json_path)) {
        int value;
        char json_value[STR_MAX];
        if (file_parseKeyValue(json_path, "frame_delay", json_value, ':', 0) != NULL) {
            value = atoi(json_value);
            // accept both microseconds and milliseconds
            frame_delay = value >= 10000 ? value / 1000 : value;
        }
    }

    // Prepare for Poll button input
    input_fd = open("/dev/input/event0", O_RDONLY);
    memset(&fds, 0, sizeof(fds));
    fds[0].fd = input_fd;
    fds[0].events = POLLIN;

    if (frame_delay < min_delay)
        frame_delay = min_delay;

    printf_debug("Frame count: %d\n", frame_count);
    printf_debug("Frame delay: %d ms\n", frame_delay);

    bool power_pressed = false;
    int repeat_power = 0;

    int current_frame = 0;

    // Set the CPU to powersave (charges faster?)
    system_powersave_on();

    uint32_t acc_ticks = 0, last_ticks = SDL_GetTicks(),
            display_timer = last_ticks;

    while (!quit) {
        while (poll(fds, 1, suspended ? 1000 - min_delay : 0)) {
            read(input_fd, &ev, sizeof(ev));

            if (ev.type != EV_KEY || ev.value > REPEATING)
                continue;

            if (ev.code == HW_BTN_POWER) {
                if (ev.value == PRESSED) {
                    power_pressed = true;
                    repeat_power = 0;
                } else if (ev.value == RELEASED && power_pressed) {
                    if (suspended) {
                        acc_ticks = 0;
                        last_ticks = SDL_GetTicks();
                    }
                    suspend(!suspended);
                    power_pressed = false;
                } else if (ev.value == REPEATING) {
                    if (repeat_power >= 5) {
                        quit = true; // power on
                        break;
                    }
                    repeat_power++;
                }
            }

            display_timer = SDL_GetTicks();
        }

        if (!battery_isCharging()) {
            quit = true;
            turn_off = true;
            break;
        }

        if (quit)
            break;

        uint32_t ticks = SDL_GetTicks();

        if (!suspended) {
            if (ticks - display_timer >= DISPLAY_TIMEOUT) {
                if (DEVICE_ID == MIYOO354) {
                    quit = true;
                    turn_off = true;
                    break;
                } else {
                    suspend(true);
                    continue;
                }
            }

            acc_ticks += ticks - last_ticks;
            last_ticks = ticks;

            if (acc_ticks >= frame_delay) {
                // Clear screen
                SDL_FillRect(screen, NULL, 0);

                if (current_frame < frame_count) {
                    SDL_Surface *frame = frames[current_frame];
                    SDL_Rect frame_rect = {320 - frame->w / 2,
                                           240 - frame->h / 2};
                    SDL_BlitSurface(frame, NULL, screen, &frame_rect);
                    current_frame = (current_frame + 1) % frame_count;
                }

                applyScreen();

                acc_ticks -= frame_delay;
            }
        }

        msleep(min_delay);
    }

#ifndef PLATFORM_MIYOOMINI
    msleep(100);
#endif

    for (int i = 0; i < frame_count; i++)
        SDL_FreeSurface(frames[i]);
    SDL_FreeSurface(screen);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    if (turn_off) {
#ifdef PLATFORM_MIYOOMINI
        display_setScreen(false);
        system("shutdown; sleep 10");
#endif
    } else {
#ifdef PLATFORM_MIYOOMINI
        display_setScreen(true);
        short_pulse();
#endif
    }

    // restore CPU performance mode
    system_powersave_off();

    return EXIT_SUCCESS;
}
