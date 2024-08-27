#ifndef STORYTELLER_APP_SELECTOR__
#define STORYTELLER_APP_SELECTOR__

#include "system/display.h"
#include <stdio.h>

#define APP_COUNT 3
#define APP_STORIES 0
#define APP_MUSIC 1
#define APP_NIGHTMODE 2

#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"
#define APP_SAVEFILE "/mnt/SDCARD/Saves/.storytellerState"

#include "./sdl_helper.h"
#include "./music_player.h"
#include "./stories_reader.h"

static char appImages[3][32] = {"selectStories.png", "selectMusic.png", "selectNightMode.png"};

static int appIndex = 0;
static bool appOpened = false;

int app_validAppIndex(int currentAppIndex, int direction) {
    int newAppIndex = currentAppIndex + direction;
    if (newAppIndex >= APP_COUNT) {
        return app_validAppIndex(-1 * direction, direction);
    }
    if (newAppIndex < 0) {
        return app_validAppIndex(APP_COUNT - 1 - direction, direction);
    }
    if(newAppIndex == APP_NIGHTMODE && parameters_getStoryDisableNightMode()) {
        return app_validAppIndex(newAppIndex, direction);
    }
    return newAppIndex;
}

void app_refreshScreen(void) {
    video_displayImage(SYSTEM_RESOURCES, appImages[appIndex]);
    display_setScreen(true);
    autosleep_unlock(parameters_getScreenOnInactivityTime(), parameters_getScreenOffInactivityTime());
}

void app_screenUpdate(void) {
    if (appOpened) {
        switch (appIndex) {
            case APP_MUSIC:
                musicplayer_screenUpdate();
                break;
            default:
                break;
        }
    }
}

void app_lockChanged(void) {
    if (appOpened) {
        switch (appIndex) {
            case APP_STORIES:
            case APP_NIGHTMODE:
                stories_lockChanged();
                break;
            case APP_MUSIC:
                musicplayer_lockChanged();
                break;
            default:
                break;
        }
    } else {
        app_refreshScreen();
    }
}

void app_menu(void) {
    if (appOpened) {
        switch (appIndex) {
            case APP_STORIES:
            case APP_NIGHTMODE:
                stories_menu();
                break;
            case APP_MUSIC:
                musicplayer_menu();
                break;
            default:
                break;
        }
    }
}

void app_previous(void) {
    if (appOpened) {
        switch (appIndex) {
            case APP_STORIES:
            case APP_NIGHTMODE:
                stories_previous();
                break;
            case APP_MUSIC:
                musicplayer_previous();
                break;
            default:
                break;
        }
    } else {
        appIndex = app_validAppIndex(appIndex, -1);
        app_refreshScreen();
    }
}

void app_next(void) {
    if (appOpened) {
        switch (appIndex) {
            case APP_STORIES:
            case APP_NIGHTMODE:
                stories_next();
                break;
            case APP_MUSIC:
                musicplayer_next();
                break;
            default:
                break;
        }
    } else {
        appIndex = app_validAppIndex(appIndex, 1);
        app_refreshScreen();
    }
}

void app_up(void) {
    if (appOpened) {
        switch (appIndex) {
            case APP_STORIES:
            case APP_NIGHTMODE:
                stories_up();
                break;
            case APP_MUSIC:
                musicplayer_up();
                break;
            default:
                break;
        }
    }
}

void app_down(void) {
    if (appOpened) {
        switch (appIndex) {
            case APP_STORIES:
            case APP_NIGHTMODE:
                stories_down();
                break;
            case APP_MUSIC:
                musicplayer_down();
                break;
            default:
                break;
        }
    }
}

void app_pause(void) {
    if (appOpened) {
        switch (appIndex) {
            case APP_STORIES:
            case APP_NIGHTMODE:
                stories_pause();
                break;
            case APP_MUSIC:
                musicplayer_pause();
                break;
            default:
                break;
        }
    }
}

void app_ok(void) {
    if (appOpened) {
        switch (appIndex) {
            case APP_STORIES:
            case APP_NIGHTMODE:
                stories_ok();
                break;
            case APP_MUSIC:
                musicplayer_ok();
                break;
            default:
                break;
        }
    } else {
        appOpened = true;
        switch (appIndex) {
            case APP_STORIES:
                stories_init();
                break;
            case APP_MUSIC:
                musicplayer_init();
                break;
            case APP_NIGHTMODE:
                stories_initNightMode();
                break;
            default:
                break;
        }
    }
}

void app_home(void) {
    if (appOpened) {
        bool appHome = true;
        switch (appIndex) {
            case APP_STORIES:
            case APP_NIGHTMODE:
                appHome = stories_home();
                break;
            case APP_MUSIC:
                appHome = musicplayer_home();
                break;
            default:
                break;
        }
        if (appHome) {
            appOpened = false;
            app_refreshScreen();
        }
    }
}

void app_save(void) {
    if (appOpened) {
        switch (appIndex) {
            case APP_STORIES:
            case APP_NIGHTMODE:
                stories_save();
                break;
            case APP_MUSIC:
                musicplayer_save();
                break;
            default:
                break;
        }
    }
}

void app_init(void) {
    cJSON *savedState = json_load(APP_SAVEFILE);
    if (json_getInt(savedState, "app", &appIndex)) {
        app_ok();
    } else {
        app_refreshScreen();
    }
}

#endif // STORYTELLER_APP_SELECTOR__