#ifndef STORYTELLER_APP_PARAMETERS__
#define STORYTELLER_APP_PARAMETERS__

#include "utils/json.h"

static double app_parameters_audioVolumeStartup = 0.3;
static double app_parameters_audioVolumeMax = 0.6;
static double app_parameters_audioSystemVolumeMax = 25.0;
static double app_parameters_screenBrightnessStartup = 0.3;
static double app_parameters_screenBrightnessMax = 0.6;
static double app_parameters_screenSystemBrightnessMax = 10.0;
static int app_parameters_screenOnInactivityTime = 120;
static int app_parameters_screenOffInactivityTime = 300;
static int app_parameters_musicInactivityTime = 3600;
static bool app_parameters_storyDisplayTiles = false;
static bool app_parameters_storyDisableNightMode = false;
static bool app_parameters_storyDisableTimeline = false;
static bool app_parameters_musicDisableRepeatModes = false;

#define APP_PARAMETERS_PATH "/mnt/SDCARD/Saves/.parameters"

int parameters_getAudioVolumeStartup() {
    return (int) (app_parameters_audioVolumeStartup * app_parameters_audioSystemVolumeMax + 0.5);
}

int parameters_getAudioVolumeMax() {
    return (int) (app_parameters_audioVolumeMax * app_parameters_audioSystemVolumeMax + 0.5);
}

int parameters_getAudioVolumeValidation(int audioVolume) {
    return audioVolume > parameters_getAudioVolumeMax() ? parameters_getAudioVolumeMax() : audioVolume;
}

int parameters_getScreenBrightnessStartup() {
    return (int) (app_parameters_screenBrightnessStartup * app_parameters_screenSystemBrightnessMax + 0.5);
}

int parameters_getScreenBrightnessMax() {
    return (int) (app_parameters_screenBrightnessMax * app_parameters_screenSystemBrightnessMax + 0.5);
}

int parameters_getScreenBrightnessValidation(int brightness) {
    return brightness > parameters_getScreenBrightnessMax() ? parameters_getScreenBrightnessMax() : brightness;
}

int parameters_getScreenOnInactivityTime() {
    return app_parameters_screenOnInactivityTime;
}

int parameters_getScreenOffInactivityTime() {
    return app_parameters_screenOffInactivityTime;
}

int parameters_getMusicInactivityTime() {
    return app_parameters_musicInactivityTime;
}

bool parameters_getStoryDisplayNine() {
    return app_parameters_storyDisplayTiles;
}

bool parameters_getStoryDisableNightMode() {
    return app_parameters_storyDisableNightMode;
}

bool parameters_getStoryDisableTimeline() {
    return app_parameters_storyDisableTimeline;
}

bool parameters_getMusicDisableRepeatModes() {
    return app_parameters_musicDisableRepeatModes;
}

void parameters_init(void) {
    cJSON *parameters = json_load(APP_PARAMETERS_PATH);
    if (parameters != NULL) {
        json_getDouble(parameters, "audioVolumeStartup", &app_parameters_audioVolumeStartup);
        json_getDouble(parameters, "audioVolumeMax", &app_parameters_audioVolumeMax);
        json_getDouble(parameters, "screenBrightnessStartup", &app_parameters_screenBrightnessStartup);
        json_getDouble(parameters, "screenBrightnessMax", &app_parameters_screenBrightnessMax);
        json_getInt(parameters, "screenOnInactivityTime", &app_parameters_screenOnInactivityTime);
        json_getInt(parameters, "screenOffInactivityTime", &app_parameters_screenOffInactivityTime);
        json_getInt(parameters, "musicInactivityTime", &app_parameters_musicInactivityTime);
        if (!cJSON_IsNull(cJSON_GetObjectItem(parameters, "storyDisplayTiles"))) {
            json_getBool(parameters, "storyDisplayTiles", &app_parameters_storyDisplayTiles);
        }
        if (!cJSON_IsNull(cJSON_GetObjectItem(parameters, "storyDisableNightMode"))) {
            json_getBool(parameters, "storyDisableNightMode", &app_parameters_storyDisableNightMode);
        }
        if (!cJSON_IsNull(cJSON_GetObjectItem(parameters, "storyDisableTimeline"))) {
            json_getBool(parameters, "storyDisableTimeline", &app_parameters_storyDisableTimeline);
        }
        if (!cJSON_IsNull(cJSON_GetObjectItem(parameters, "musicDisableRepeatModes"))) {
            json_getBool(parameters, "musicDisableRepeatModes", &app_parameters_musicDisableRepeatModes);
        }
    }
}

#endif // STORYTELLER_APP_PARAMETERS__