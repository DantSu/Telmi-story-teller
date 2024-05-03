#ifndef STORYTELLER_APP_PARAMETERS__
#define STORYTELLER_APP_PARAMETERS__

#include "utils/json.h"

static double parametersAudioVolumeStartup = 0.3;
static double parametersAudioVolumeMax = 0.6;
static double parametersScreenBrightnessStartup = 0.3;
static double parametersScreenBrightnessMax = 0.6;
static int parametersScreenOnInactivityTime = 120;
static int parametersScreenOffInactivityTime = 300;
static int parametersMusicInactivityTime = 3600;
static bool parametersStoryDisplayTiles = false;

#define APP_PARAMETERS_PATH "/mnt/SDCARD/Saves/.parameters"


int parameters_getAudioVolumeStartup() {
    return (int)(parametersAudioVolumeStartup * 20 + 0.5);
}

int parameters_getAudioVolumeMax() {
    return (int)(parametersAudioVolumeMax * 20 + 0.5);
}

int parameters_getAudioVolumeValidation(int audioVolume) {
    return audioVolume > parameters_getAudioVolumeMax() ? parameters_getAudioVolumeMax() : audioVolume;
}

int parameters_getScreenBrightnessStartup() {
    return (int)(parametersScreenBrightnessStartup * 10 + 0.5);
}

int parameters_getScreenBrightnessMax() {
    return (int)(parametersScreenBrightnessMax * 10 + 0.5);
}

int parameters_getScreenBrightnessValidation(int brightness) {
    return brightness > parameters_getScreenBrightnessMax() ? parameters_getScreenBrightnessMax() : brightness;
}

int parameters_getScreenOnInactivityTime() {
    return parametersScreenOnInactivityTime;
}

int parameters_getScreenOffInactivityTime() {
    return parametersScreenOffInactivityTime;
}

int parameters_getMusicInactivityTime() {
    return parametersMusicInactivityTime;
}

bool parameters_getStoryDisplayNine() {
    return parametersStoryDisplayTiles;
}

void parameters_init(void)
{
    cJSON *parameters = json_load(APP_PARAMETERS_PATH);
    if(parameters != NULL) {
        json_getDouble(parameters, "audioVolumeStartup", &parametersAudioVolumeStartup);
        json_getDouble(parameters, "audioVolumeMax", &parametersAudioVolumeMax);
        json_getDouble(parameters, "screenBrightnessStartup", &parametersScreenBrightnessStartup);
        json_getDouble(parameters, "screenBrightnessMax", &parametersScreenBrightnessMax);
        json_getInt(parameters, "screenOnInactivityTime", &parametersScreenOnInactivityTime);
        json_getInt(parameters, "screenOffInactivityTime", &parametersScreenOffInactivityTime);
        json_getInt(parameters, "musicInactivityTime", &parametersMusicInactivityTime);
        if(!cJSON_IsNull(cJSON_GetObjectItem(parameters, "storyDisplayTiles"))) {
            json_getBool(parameters, "storyDisplayTiles", &parametersStoryDisplayTiles);
        }
    }
}

#endif // STORYTELLER_APP_PARAMETERS__