#ifndef STORYTELLER_MUSICPLAYER_HELPER__
#define STORYTELLER_MUSICPLAYER_HELPER__

#include <string.h>
#include <dirent.h>
#include "system/display.h"
#include "utils/str.h"

#include "./app_autosleep.h"
#include "./sdl_helper.h"

static char **musicList = NULL;
static int musicCount = 0;
static int musicIndex = 0;
static void (*callback_musicplayer_autoplay)(void);

#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"
#define MUSICPLAYER_RESOURCES "/mnt/SDCARD/Music/"


void musicplayer_load(void)
{
    if(musicCount == 0) {
        video_displayImage(SYSTEM_RESOURCES, "noMusic.png");
        return;
    }

    if(musicIndex < 0) {
        musicIndex = musicCount - 1;
    } else if (musicIndex >= musicCount) {
        musicIndex = 0;
    }
    
    video_displayBlackScreen();
    display_setScreen(false);
    autosleep_lock();
    audio_play(MUSICPLAYER_RESOURCES, musicList[musicIndex], 0);
    Mix_HookMusicFinished(callback_musicplayer_autoplay);
}

void musicplayer_next(void)
{
    musicIndex += 1;
    musicplayer_load();
}

void musicplayer_previous(void)
{
    musicIndex -= 1;
    musicplayer_load();
}

void musicplayer_ok(void)
{
    
}

void musicplayer_autoplay(void)
{
    musicplayer_next();
}


void musicplayer_pause(void)
{
    if(Mix_PlayingMusic() == 1) {
        if (Mix_PausedMusic() == 1) {
            autosleep_lock();
            Mix_ResumeMusic();
        } else {
            autosleep_unlock();
            Mix_PauseMusic();
        }
    }
}

bool musicplayer_home(void)
{
    Mix_HookMusicFinished(NULL);
    if(Mix_PlayingMusic() == 1) {
        if (Mix_PausedMusic() != 1) {
            Mix_PauseMusic();
        }
    }
    return true;
}

void musicplayer_save(void)
{
    
}

bool musicplayer_isMp3File(const char *fileName)
{
    return strcmp((char *)fileName + strlen(fileName) - 4, ".mp3") == 0 || strcmp((char *)fileName + strlen(fileName) - 4, ".MP3") == 0;
}

void musicplayer_init(void)
{
    callback_musicplayer_autoplay = &musicplayer_autoplay;

    video_displayImage(SYSTEM_RESOURCES, "loadingMusic.png");

    int i=0;
    DIR *d;
    struct dirent *dir;
    d = opendir(MUSICPLAYER_RESOURCES);
    
    while((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_REG && musicplayer_isMp3File(dir->d_name)) {
            i++;
        }
    }
    rewinddir(d);

    musicCount = i;
    char **filesList = (char**)malloc(i * sizeof(char*));
    i=0;

    while((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_REG && musicplayer_isMp3File(dir->d_name)) {
            filesList[i] = (char*) malloc(STR_MAX * sizeof(char));
            strcpy(filesList[i], dir->d_name);
            i++;
        }
    }
    closedir(d);
    musicList = filesList;
    musicplayer_load();
}

#endif // STORYTELLER_MUSICPLAYER_HELPER__