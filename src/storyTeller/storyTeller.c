#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "system/osd.h"
#include "system/system.h"
#include "system/keymap_hw.h"
#include "system/settings.h"
#include "system/settings_sync.h"
#include "utils/log.h"

#include "./sdl_helper.h"
#include "./app_selector.h"


// for ev.value
#define RELEASED 0
#define PRESSED 1
#define REPEAT 2

// Global Variables
static int input_fd;
static struct input_event ev;
static struct pollfd fds[1];

bool keyinput_isValid(void)
{
    read(input_fd, &ev, sizeof(ev));

    if (ev.type != EV_KEY || ev.value > REPEAT) {
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    video_audio_init();
    settings_init();
    display_init();
    settings_setVolume(6, true);
    settings_setBrightness(3, true, false);

    app_init();

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
                        case HW_BTN_LEFT :
                            app_previous();
                            break;
                        case HW_BTN_RIGHT :
                            app_next();
                            break;
                        case HW_BTN_START :
                        case HW_BTN_SELECT :
                            app_pause();
                            break;
                        case HW_BTN_A :
                        case HW_BTN_B :
                            app_ok();
                            break;
                        case HW_BTN_Y :
                        case HW_BTN_X :
                            app_home();
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
    video_audio_quit();
    system_shutdown();
    return EXIT_SUCCESS;
}
