#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "system/osd.h"
#include "system/system.h"
#include "system/keymap_hw.h"
#include "system/settings.h"
#include "system/settings_sync.h"
#include "system/display.h"

#include "./logs_helper.h"
#include "./time_helper.h"
#include "./app_lock.h"
#include "./app_autosleep.h"
#include "./sdl_helper.h"
#include "./app_selector.h"
#include "./app_parameters.h"

// for ev.value
#define RELEASED 0
#define PRESSED 1
#define REPEAT 2

// Global Variables
static int input_fd;
static struct input_event ev;
static struct pollfd fds[1];

bool keyinput_isValid(void) {
    read(input_fd, &ev, sizeof(ev));

    if (ev.type != EV_KEY || ev.value > REPEAT) {
        return false;
    }

    return true;
}

int main(int argc, char *argv[]) {

    srand(time(NULL));
    video_audio_init();
    settings_init();
    display_init();
    parameters_init();
    settings_setVolume(parameters_getAudioVolumeStartup(), true);
    settings_setBrightness(parameters_getScreenBrightnessStartup(), true, false);

    autosleep_init(parameters_getScreenOnInactivityTime(), parameters_getScreenOffInactivityTime());
    app_init();

    input_fd = open("/dev/input/event0", O_RDONLY);
    memset(&fds, 0, sizeof(fds));
    fds[0].fd = input_fd;
    fds[0].events = POLLIN;

    bool isMenuPressed = false;
    bool menuPreventDefault = false;
    bool startPowerPressed = false;
    long startPowerPressedTime = 0;

    while (1) {
        if (autosleep_isSleepingTime() || (startPowerPressed && (get_time() - startPowerPressedTime) > 1)) {
            goto exit_loop;
        }

        applock_checkLock();
        app_screenUpdate();

        if (poll(fds, 1, 0) > 0) {
            if (!keyinput_isValid()) {
                continue;
            }

            switch (ev.value) {
                case PRESSED:
                    switch (ev.code) {
                        case HW_BTN_MENU :
                            isMenuPressed = true;
                            applock_startTimer();
                            if (applock_isLocked()) {
                                menuPreventDefault = true;
                            }
                            break;
                        case HW_BTN_POWER :
                            if (!applock_isLocked()) {
                                startPowerPressedTime = get_time();
                                startPowerPressed = true;
                            }
                            break;
                    }
                    break;

                case RELEASED:
                    if (applock_isLocked()) {
                        if (ev.code == HW_BTN_MENU) {
                            applock_stopTimer();
                        }
                        break;
                    }
                    autosleep_keepAwake();
                    switch (ev.code) {
                        case HW_BTN_POWER :
                            startPowerPressed = false;
                            break;
                        case HW_BTN_MENU :
                            if (!menuPreventDefault) {
                                app_menu();
                            }
                            isMenuPressed = false;
                            menuPreventDefault = false;
                            applock_stopTimer();
                            break;
                        case HW_BTN_LEFT :
                            app_previous();
                            break;
                        case HW_BTN_RIGHT :
                            app_next();
                            break;
                        case HW_BTN_UP :
                            app_up();
                            break;
                        case HW_BTN_DOWN :
                            app_down();
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
                    }

                    if (isMenuPressed) {
                        switch (ev.code) {
                            case HW_BTN_L2 :
                            case HW_BTN_VOLUME_DOWN :
                                settings_setBrightness(settings.brightness - 1, true, false);
                                osd_showBrightnessBar(settings.brightness);
                                applock_stopTimer();
                                menuPreventDefault = true;
                                break;
                            case HW_BTN_R2 :
                            case HW_BTN_VOLUME_UP :
                                settings_setBrightness(
                                        parameters_getScreenBrightnessValidation(settings.brightness + 1),
                                        true,
                                        false);
                                osd_showBrightnessBar(settings.brightness);
                                applock_stopTimer();
                                menuPreventDefault = true;
                                break;
                            default:
                                break;
                        }
                    } else {
                        switch (ev.code) {
                            case HW_BTN_VOLUME_DOWN :
                                settings_setVolume(settings.volume - 1, true);
                                osd_showVolumeBar(settings.volume, false);
                                break;
                            case HW_BTN_VOLUME_UP :
                                settings_setVolume(parameters_getAudioVolumeValidation(settings.volume + 1), true);
                                osd_showVolumeBar(settings.volume, false);
                                break;
                            default:
                                break;
                        }
                    }
                    break;

                default:
                    break;
            }
        }
    }

    exit_loop:
    app_save();
    display_setScreen(true);
    video_audio_quit();
    system_shutdown();
    return EXIT_SUCCESS;
}
