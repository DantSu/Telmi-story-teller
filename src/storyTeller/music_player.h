#ifndef STORYTELLER_MUSICPLAYER_HELPER__
#define STORYTELLER_MUSICPLAYER_HELPER__

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include "system/display.h"
#include "utils/str.h"

#include "./app_autosleep.h"
#include "./sdl_helper.h"
#include "./app_parameters.h"
#include "./array_helper.h"
#include "./time_helper.h"

static char **musicList = NULL;
static int *musicAlbumIndex = NULL;
static int musicCount = 0;
static int musicAlbumCount = 0;
static int musicIndex = 0;
static int musicStartTime = 0;
static int musicPosition = 0;
static long int musicScreenUpdate = 0;
static long int musicLastActivity = 0;
static void (*callback_musicplayer_autoplay)(void);

#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"
#define MUSICPLAYER_RESOURCES "/mnt/SDCARD/Music/"

void musicplayer_autosleep_unlock(void) {
    autosleep_unlock(parameters_getScreenOnInactivityTime(), parameters_getScreenOffInactivityTime());
}

void musicplayer_autosleep_lock(void) {
    autosleep_unlock(parameters_getMusicInactivityTime(), parameters_getMusicInactivityTime());
}

void musicplayer_drawSideMusic(int index, int top) {
    int mIndex = musicIndex + index;

    if(mIndex < 0) {
        mIndex = musicCount + mIndex;
    } else if (mIndex >= musicCount) {
        mIndex = mIndex - musicCount;
    }
    
    char fileImageName[STR_MAX], writeTitle[STR_MAX], writeArtist[STR_MAX],
    imageName[STR_MAX - 4], imageNameCopy[STR_MAX - 4], imageNameDelimiter[] = "_";
    
    int length = strlen(musicList[mIndex]) - 4;
    strncpy(imageName, musicList[mIndex], length);
    imageName[length] = '\0';
    strcpy(imageNameCopy, imageName);

    char *artist = strtok(imageNameCopy, imageNameDelimiter);
    char *album = strtok(NULL, imageNameDelimiter);
    char *track = strtok(NULL, imageNameDelimiter);
    char *title = strtok(NULL, imageNameDelimiter);

    sprintf(writeTitle, "%s. %s", track, title);
    sprintf(writeArtist, "%s - %s", artist, album);
    sprintf(fileImageName, "%s.png", imageName);

    video_screenAddImage(MUSICPLAYER_RESOURCES, fileImageName, 55, top + 9, 64);
    video_screenWriteFont(writeTitle, fontBold20, colorWhite60, 150, top + 17, SDL_ALIGN_LEFT);
    video_screenWriteFont(writeArtist, fontRegular16, colorWhite60, 150, top + 43, SDL_ALIGN_LEFT);
}

void musicplayer_drawInterface(int displayMusicPosition) {
    char fileImageName[STR_MAX], writeTitle[STR_MAX], writeArtist[STR_MAX], writeDuration[STR_MAX], writeTime[STR_MAX],
    imageName[STR_MAX - 4], imageNameCopy[STR_MAX - 4], imageNameDelimiter[] = "_";
    
    int length = strlen(musicList[musicIndex]) - 4;
    strncpy(imageName, musicList[musicIndex], length);
    imageName[length] = '\0';
    strcpy(imageNameCopy, imageName);

    char *artist = strtok(imageNameCopy, imageNameDelimiter);
    char *album = strtok(NULL, imageNameDelimiter);
    char *track = strtok(NULL, imageNameDelimiter);
    char *title = strtok(NULL, imageNameDelimiter);

    int musicDuration = audio_getDuration();
    musicScreenUpdate = get_time();

    sprintf(writeTitle, "%s. %s", track, title);
    sprintf(writeArtist, "%s - %s", artist, album);
    sprintf(writeDuration, "%i:%02i", musicDuration / 60, musicDuration % 60);
    sprintf(writeTime, "%i:%02i", displayMusicPosition / 60, displayMusicPosition % 60);
    sprintf(fileImageName, "%s.png", imageName);

    video_screenBlack();
    video_drawRectangle(185, 258, (int)((double)displayMusicPosition * 422.0 / (double)musicDuration), 12, 255, 186, 0);
    video_screenAddImage(SYSTEM_RESOURCES, "musicPlayer.png", 0, 0, 640);
    video_screenAddImage(MUSICPLAYER_RESOURCES, fileImageName, 24, 176, 128);
    video_screenWriteFont(writeTitle, fontBold24, colorWhite, 185, 190, SDL_ALIGN_LEFT);
    video_screenWriteFont(writeArtist, fontRegular20, colorWhite, 185, 222, SDL_ALIGN_LEFT);
    video_screenWriteFont(writeTime, fontRegular18, colorWhite, 185, 275, SDL_ALIGN_LEFT);
    video_screenWriteFont(writeDuration, fontRegular18, colorWhite, 605, 275, SDL_ALIGN_RIGHT);
    musicplayer_drawSideMusic(-2, 0);
    musicplayer_drawSideMusic(-1, 83);
    musicplayer_drawSideMusic(1, 314);
    musicplayer_drawSideMusic(2, 397);
    video_applyToVideo();
}

void musicplayer_screenUpdate(void) {
    if(!display_enabled || musicList == NULL) {
        return;
    }

    long int ts = get_time();

    if(musicScreenUpdate != ts) {
        int inactivityTime = ts - musicLastActivity;

        if(inactivityTime > 10) {
            video_displayBlackScreen();
            display_setScreen(false);
            return;
        }

        int mPos = musicPosition;
        if(Mix_PausedMusic() != 1) {
            mPos += ts - musicStartTime;
        }
        musicplayer_drawInterface(mPos);
    }
}

void musicplayer_screenActivate(void) {
    musicLastActivity = get_time();
    display_setScreen(true);
}

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

    musicStartTime = get_time();
    audio_play(MUSICPLAYER_RESOURCES, musicList[musicIndex], (double*)&musicPosition);
    Mix_HookMusicFinished(callback_musicplayer_autoplay);

    if(display_enabled) {
        musicplayer_drawInterface(musicPosition);
    }
}

void musicplayer_up(void)
{
    musicplayer_screenActivate();
    musicPosition = 0;
    musicIndex -= 1;
    musicplayer_load();
}

void musicplayer_down(void)
{
    musicplayer_screenActivate();
    musicPosition = 0;
    musicIndex += 1;
    musicplayer_load();
}

void musicplayer_rewind(int time)
{
    musicplayer_screenActivate();
    int musicDuration = audio_getDuration();
    long int ts = get_time();
    musicPosition += ts - musicStartTime + time;
    musicStartTime = ts;
    if (musicPosition < 0) {
        musicPosition = 0;
    } else if (musicPosition >= musicDuration) {
        musicPosition = musicDuration - 1;
    }
    Mix_SetMusicPosition((double)musicPosition);
    musicplayer_drawInterface(musicPosition);
}

void musicplayer_next(void)
{
    musicplayer_rewind(10);
}

void musicplayer_previous(void)
{
    musicplayer_rewind(-10);
}

void musicplayer_ok(void)
{
    musicplayer_screenActivate();
}

void musicplayer_autoplay(void)
{
    musicPosition = 0;
    musicIndex += 1;
    musicplayer_load();
}


void musicplayer_pause(void)
{
    musicplayer_screenActivate();
    if(Mix_PlayingMusic() == 1) {
        if (Mix_PausedMusic() == 1) {
            Mix_ResumeMusic();
            musicStartTime = get_time();
            Mix_SetMusicPosition((double)musicPosition);
            musicplayer_autosleep_lock();
        } else {
            Mix_PauseMusic();
            musicPosition += get_time() - musicStartTime;
            musicplayer_autosleep_unlock();
        }
    }
}

bool musicplayer_home(void)
{
    audio_free_music();
    return true;
}

void musicplayer_save(void)
{
    
}

bool musicplayer_isMp3File(const char *fileName)
{
    return strcmp((char *)fileName + strlen(fileName) - 4, ".mp3") == 0;
}

bool musicplayer_isNewAlbum(const char* fileName, char* lastAlbum) {
    char *pointer = strchr(fileName, '_');
    pointer = strchr(pointer + 1, '_');
    char albumName[STR_MAX];
    strncpy(albumName, fileName, pointer - fileName);
    if(strcmp(albumName, lastAlbum) == 0) {
        return false;
    }
    strcpy(lastAlbum, albumName);
    return true;
}

void musicplayer_init(void)
{
    musicplayer_autosleep_lock();
    musicplayer_screenActivate();

    if (musicList != NULL) {
        return musicplayer_load();
    }

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
    musicAlbumCount = 0;
    char **filesList = (char**)malloc(musicCount * sizeof(char*));
    char lastAlbum[STR_MAX] = {'\0'};
    i=0;

    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_REG && musicplayer_isMp3File(dir->d_name)) {
            filesList[i] = malloc(STR_MAX);
            strcpy(filesList[i], dir->d_name);

            if (musicplayer_isNewAlbum(dir->d_name, lastAlbum)) {
                musicAlbumCount++;
            }

            i++;
        }
    }
    closedir(d);

    if (musicCount > 0) {
        sort(filesList, musicCount); 
    }

    musicAlbumIndex = malloc(sizeof(int) * musicAlbumCount);
    lastAlbum[0] = '\0';
    int j = 0;
    for (i = 0; i < musicCount; i++)
    {
        if (musicplayer_isNewAlbum(filesList[i], lastAlbum)) {
            musicAlbumIndex[j] = i;
            j++;
        }
    }

    musicList = filesList;
    musicplayer_load();
}

#endif // STORYTELLER_MUSICPLAYER_HELPER__