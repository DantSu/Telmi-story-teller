#ifndef STORYTELLER_APP_VOLUME__
#define STORYTELLER_APP_VOLUME__

#include "system/settings.h"
#include "./app_parameters.h"
#include "./time_helper.h"

static long int app_volume_showedTime = 0;
static bool app_volume_showed = false;

bool app_volume_isShowed(void) {
    return app_volume_showed;
}

bool app_volume_show(void) {
    app_volume_showedTime = get_time() + 2;
    app_volume_showed = true;
    return true;
}

int app_volume_getCurrent(void) {
    return settings.volume;
}

int app_volume_getMax(void) {
    return app_parameters_audioSystemVolumeMax;
}

bool app_volume_up(void) {
    settings_setVolume(parameters_getAudioVolumeValidation(app_volume_getCurrent() + 1), true);
    return app_volume_show();
}

bool app_volume_down(void) {
    settings_setVolume(app_volume_getCurrent() - 1, true);
    return app_volume_show();
}

bool app_volume_checkDisplay(void) {
    if(app_volume_showed && app_volume_showedTime <= get_time()) {
        app_volume_showed = false;
        return true;
    }
    return false;
}

#endif // STORYTELLER_APP_VOLUME__