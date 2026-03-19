#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "system/system.h"
#include "system/settings.h"
#include "system/settings_sync.h"
#include "system/display.h"

#include "./logs_helper.h"
#include "./time_helper.h"
#include "./app_lock.h"
#include "./app_brightness.h"
#include "./app_volume.h"
#include "./app_autosleep.h"
#include "./sdl_helper.h"
#include "./app_selector.h"
#include "./app_parameters.h"
#include "platform_input.h"

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

    platform_input_init();

    bool isMenuPressed = false;
    bool menuPreventDefault = false;
    bool startPowerPressed = false;
    long startPowerPressedTime = 0;
    bool shouldPowerOff = false;

    while (1) {
        if (autosleep_isSleepingTime() || (startPowerPressed && (get_time() - startPowerPressedTime) > 1)) {
            shouldPowerOff = true;
            goto exit_loop;
        }

        bool forceRefreshScreen = applock_checkLock();
        forceRefreshScreen = app_volume_checkDisplay() || forceRefreshScreen;
        forceRefreshScreen = app_brightness_checkDisplay() || forceRefreshScreen;
        app_update();

        InputAction action;
        bool isPressed;
        while (platform_input_poll(&action, &isPressed)) {
            if (action == INPUT_ACTION_QUIT) {
                goto exit_loop;
            }

            if (isPressed) {
                switch (action) {
                    case INPUT_ACTION_MENU:
                        isMenuPressed = true;
                        forceRefreshScreen = applock_startTimer() || forceRefreshScreen;
                        if (applock_isLocked()) {
                            menuPreventDefault = true;
                        }
                        break;
                    case INPUT_ACTION_POWER:
                        if (!applock_isLocked()) {
                            startPowerPressedTime = get_time();
                            startPowerPressed = true;
                        }
                        break;
                    default:
                        break;
                }
            } else {
                if (applock_isLocked()) {
                    if (action == INPUT_ACTION_MENU) {
                        forceRefreshScreen = applock_stopTimer() || forceRefreshScreen;
                    }
                    continue;
                }
                autosleep_keepAwake();
                switch (action) {
                    case INPUT_ACTION_POWER:
                        startPowerPressed = false;
                        break;
                    case INPUT_ACTION_MENU:
                        if (!menuPreventDefault) {
                            app_menu();
                        }
                        isMenuPressed = false;
                        menuPreventDefault = false;
                        forceRefreshScreen = applock_stopTimer() || forceRefreshScreen;
                        break;
                    case INPUT_ACTION_LEFT:
                        app_previous();
                        break;
                    case INPUT_ACTION_RIGHT:
                        app_next();
                        break;
                    case INPUT_ACTION_UP:
                        app_up();
                        break;
                    case INPUT_ACTION_DOWN:
                        app_down();
                        break;
                    case INPUT_ACTION_START:
                    case INPUT_ACTION_SELECT:
                        app_pause();
                        break;
                    case INPUT_ACTION_A:
                    case INPUT_ACTION_B:
                        app_ok();
                        break;
                    case INPUT_ACTION_Y:
                    case INPUT_ACTION_X:
                        app_home();
                        break;
                    default:
                        break;
                }

                if (isMenuPressed) {
                    switch (action) {
                        case INPUT_ACTION_L2:
                        case INPUT_ACTION_VOL_DOWN:
                            forceRefreshScreen = app_brightness_down();
                            applock_stopTimer();
                            menuPreventDefault = true;
                            break;
                        case INPUT_ACTION_R2:
                        case INPUT_ACTION_VOL_UP:
                            forceRefreshScreen = app_brightness_up();
                            applock_stopTimer();
                            menuPreventDefault = true;
                            break;
                        default:
                            break;
                    }
                } else {
                    switch (action) {
                        case INPUT_ACTION_VOL_DOWN:
                            forceRefreshScreen = app_volume_down();
                            break;
                        case INPUT_ACTION_VOL_UP:
                            forceRefreshScreen = app_volume_up();
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        if(forceRefreshScreen) {
            app_forceRefreshScreen();
        }
    }

    exit_loop:
    app_save();
    platform_input_quit();
    display_setScreen(true);
    video_audio_quit();
    if (shouldPowerOff) {
        system_shutdown();
    }
    return EXIT_SUCCESS;
}
