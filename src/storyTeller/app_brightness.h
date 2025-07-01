#ifndef STORYTELLER_APP_BRIGHTNESS__
#define STORYTELLER_APP_BRIGHTNESS__

#include "system/settings.h"
#include "./app_parameters.h"
#include "./time_helper.h"

static long int app_brightness_showedTime = 0;
static bool app_brightness_showed = false;

bool app_brightness_isShowed(void) {
    return app_brightness_showed;
}

bool app_brightness_show(void) {
    app_brightness_showedTime = get_time() + 2;
    app_brightness_showed = true;
    return true;
}

int app_brightness_getCurrent(void) {
    return settings.brightness;
}

int app_brightness_getMax(void) {
    return app_parameters_screenSystemBrightnessMax;
}

bool app_brightness_up(void) {
    settings_setBrightness(
            parameters_getScreenBrightnessValidation(app_brightness_getCurrent() + 1),
            true,
            false);
    return app_brightness_show();
}

bool app_brightness_down(void) {
    settings_setBrightness(app_brightness_getCurrent() - 1, true, false);
    return app_brightness_show();
}

bool app_brightness_checkDisplay(void) {
    if(app_brightness_showed && app_brightness_showedTime <= get_time()) {
        app_brightness_showed = false;
        return true;
    }
    return false;
}


#endif // STORYTELLER_APP_BRIGHTNESS__