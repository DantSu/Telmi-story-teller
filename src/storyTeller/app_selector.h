#ifndef STORYTELLER_APP_SELECTOR__
#define STORYTELLER_APP_SELECTOR__

#include "system/display.h"

#include "./sdl_helper.h"
#include "./music_player.h"
#include "./stories_reader.h"


#define APP_COUNT 2
#define APP_STORIES 0
#define APP_MUSIC 1


#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"
static char appImages[2][32] = {"selectStories.png", "selectMusic.png"};

static int appIndex = 0;
static bool appOpened = false;

void app_refreshScreen(void)
{
    if(appIndex >= APP_COUNT) {
        appIndex = 0;
    } else if (appIndex < 0) {
        appIndex = APP_COUNT - 1;
    }
    video_displayImage(SYSTEM_RESOURCES, appImages[appIndex]);
    display_setScreen(true);
    autosleep_unlock();
}

void app_init(void)
{
    app_refreshScreen();
}

void app_previous(void)
{
    if(appOpened) {
        switch (appIndex)
        {
            case APP_STORIES:
                stories_previous();
                break;
            case APP_MUSIC:
                musicplayer_previous();
                break;
            default:
                break;
        }
    } else {
        appIndex -= 1;
        app_refreshScreen();
    }
}

void app_next(void)
{
    if(appOpened) {
        switch (appIndex)
        {
            case APP_STORIES:
                stories_next();
                break;
            case APP_MUSIC:
                musicplayer_next();
                break;
            default:
                break;
        }
    } else {
        appIndex += 1;
        app_refreshScreen();
    }
}

void app_pause(void)
{
    if(appOpened) {
        switch (appIndex)
        {
            case APP_STORIES:
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

void app_ok(void)
{
    if(appOpened) {
        switch (appIndex)
        {
            case APP_STORIES:
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
        switch (appIndex)
        {
            case APP_STORIES:
                stories_init();
                break;
            case APP_MUSIC:
                musicplayer_init();
                break;
            default:
                break;
        }
    }
}

void app_home(void)
{
    if(appOpened) {
        bool appHome = true;
        switch (appIndex)
        {
            case APP_STORIES:
                appHome = stories_home();
                break;
            case APP_MUSIC:
                appHome = musicplayer_home();
                break;
            default:
                break;
        }
        if(appHome) {
            appOpened = false;
            app_refreshScreen();
        }
    }
}


#endif // STORYTELLER_APP_SELECTOR__