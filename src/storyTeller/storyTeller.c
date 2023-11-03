#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "system/osd.h"
#include "system/system.h"
#include "system/keymap_hw.h"
#include "system/settings.h"
#include "system/settings_sync.h"
#include "utils/file.h"
#include "utils/log.h"

// for ev.value
#define RELEASED 0
#define PRESSED 1
#define REPEAT 2

// Global Variables
static int input_fd;
static struct input_event ev;
static struct pollfd fds[1];

#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"
#define STORIES_RESOURCES "/mnt/SDCARD/StoryTeller/"

SDL_Surface *loadImage(const char *dir, const char *name)
{
    char image_path[512];
    sprintf(image_path, "%s%s.png", dir, name);
    return IMG_Load(image_path);
}

bool keyinput_isValid(void)
{
    read(input_fd, &ev, sizeof(ev));

    if (ev.type != EV_KEY || ev.value > REPEAT)
        return false;

    return true;
}

int main(int argc, char *argv[])
{
    settings_init();
    display_init();
    setVolume(settings.volume);
    display_setBrightness(settings.brightness);


    SDL_Init(SDL_INIT_VIDEO);

    SDL_Surface *video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    SDL_Surface *screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
    SDL_Surface *background = loadImage(SYSTEM_RESOURCES, "storiesLoading");

    SDL_BlitSurface(background, NULL, screen, NULL);
    SDL_FreeSurface(background);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    // Prepare for Poll button input
    input_fd = open("/dev/input/event0", O_RDONLY);
    memset(&fds, 0, sizeof(fds));
    fds[0].fd = input_fd;
    fds[0].events = POLLIN;

    bool is_menu_pressed = false;

    while (1) {
        if (poll(fds, 1, 0) > 0) {
            if (!keyinput_isValid()) {
                continue;
            }

            switch (ev.value) {
                case PRESSED:
                    if(ev.code == HW_BTN_MENU) {
                        is_menu_pressed = true;
                    }
                    break;
                
                case RELEASED:
                    switch (ev.code)
                    {
                        case HW_BTN_POWER :
                            goto exit_loop;
                            break;
                        case HW_BTN_MENU :
                            is_menu_pressed = false;
                            break;
                        case HW_BTN_A :
                            
                            break;
                        case HW_BTN_B :
                            
                            break;
                        case HW_BTN_VOLUME_DOWN :
                            if(is_menu_pressed) {
                                settings_setBrightness(settings.brightness - 1, true, false);
                                osd_showBrightnessBar(settings.brightness);
                            } else {
                                settings_setVolume(settings.volume - 1, true);
                                osd_showVolumeBar(settings.volume, false);
                            }
                            break;
                        case HW_BTN_VOLUME_UP :
                            if(is_menu_pressed) {
                                settings_setBrightness(settings.brightness + 1, true, false);
                                osd_showBrightnessBar(settings.brightness);
                            } else {
                                settings_setVolume(settings.volume + 1, true);
                                osd_showVolumeBar(settings.volume, false);
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                
                default:
                    break;
            }
        }
    }
    
    exit_loop:
    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    SDL_Quit();

    system_shutdown();

    return EXIT_SUCCESS;
}
