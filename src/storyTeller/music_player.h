#ifndef STORYTELLER_MUSICPLAYER_HELPER__
#define STORYTELLER_MUSICPLAYER_HELPER__

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include "system/display.h"
#include "utils/str.h"
#include "utils/json.h"

#include "./app_lock.h"
#include "./app_file.h"
#include "./app_autosleep.h"
#include "./sdl_helper.h"
#include "./app_parameters.h"
#include "./array_helper.h"
#include "./time_helper.h"

#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"
#define MUSICPLAYER_RESOURCES "/mnt/SDCARD/Music/"

#define MUSICPLAYER_MODE_PLAYER 0
#define MUSICPLAYER_MODE_ALBUM 1

#define MUSICPLAYER_REPEAT_ALL 0
#define MUSICPLAYER_REPEAT_SHUFFLE 1
#define MUSICPLAYER_REPEAT_ALBUM 2
#define MUSICPLAYER_REPEAT_TITLE 3

static int musicPlayerMode = MUSICPLAYER_MODE_PLAYER;
static char **musicPlayerTracksList = NULL;
static int musicPlayerTracksCount = 0;
static int musicPlayerTrackIndex = 0;
static int musicPlayerTrackStartTime = 0;
static int musicPlayerTrackPosition = 0;
static int *musicPlayerAlbumsIndex = NULL;
static int musicPlayerAlbumsCount = 0;
static int musicPlayerAlbumIndex = 0;
static int musicPlayerRepeatMode = MUSICPLAYER_REPEAT_ALL;
static long int musicPlayerScreenUpdate = 0;
static long int musicPlayerLastActivity = 0;

static void (*callback_musicplayer_autoplay)(void);


void musicplayer_saveSession(void) {
    file_save(
            APP_SAVEFILE,
            "{\"app\":%d, \"musicIndex\":%d, \"musicPosition\":%d}",
            APP_MUSIC,
            musicPlayerTrackIndex,
            musicPlayerTrackPosition
    );
}

void musicplayer_loadSession(void) {
    cJSON *savedState = json_load(APP_SAVEFILE);
    int a;
    if (savedState != NULL && json_getInt(savedState, "app", &a) && a == APP_MUSIC) {
        json_getInt(savedState, "musicIndex", &musicPlayerTrackIndex);
        json_getInt(savedState, "musicPosition", &musicPlayerTrackPosition);
        remove(APP_SAVEFILE);
    }
}

void musicplayer_autosleep_unlock(void) {
    autosleep_unlock(parameters_getScreenOnInactivityTime(), parameters_getScreenOffInactivityTime());
}

void musicplayer_autosleep_lock(void) {
    autosleep_unlock(parameters_getMusicInactivityTime(), parameters_getMusicInactivityTime());
}

void musicplayer_interfaceplayer_drawSideMusic(int index, int top) {
    int mIndex = musicPlayerTrackIndex + index;

    if (mIndex < 0) {
        mIndex = musicPlayerTracksCount + mIndex;
    } else if (mIndex >= musicPlayerTracksCount) {
        mIndex = mIndex - musicPlayerTracksCount;
    }

    if (mIndex < 0 || mIndex >= musicPlayerTracksCount) {
        return;
    }

    char fileImageName[STR_MAX], writeTitle[STR_MAX], writeArtist[STR_MAX],
            imageName[STR_MAX - 4], imageNameCopy[STR_MAX - 4], imageNameDelimiter[] = "_";

    int length = strlen(musicPlayerTracksList[mIndex]) - 4;
    strncpy(imageName, musicPlayerTracksList[mIndex], length);
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

void musicplayer_interfaceplayer_drawInterface(int displayMusicPosition) {
    char fileImageName[STR_MAX], writeTitle[STR_MAX], writeArtist[STR_MAX], writeDuration[STR_MAX], writeTime[STR_MAX],
            imageName[STR_MAX - 4], imageNameCopy[STR_MAX - 4], imageNameDelimiter[] = "_";

    int length = strlen(musicPlayerTracksList[musicPlayerTrackIndex]) - 4;
    strncpy(imageName, musicPlayerTracksList[musicPlayerTrackIndex], length);
    imageName[length] = '\0';
    strcpy(imageNameCopy, imageName);

    char *artist = strtok(imageNameCopy, imageNameDelimiter);
    char *album = strtok(NULL, imageNameDelimiter);
    char *track = strtok(NULL, imageNameDelimiter);
    char *title = strtok(NULL, imageNameDelimiter);

    int musicDuration = audio_getDuration();
    musicPlayerScreenUpdate = get_time();

    sprintf(writeTitle, "%s. %s", track, title);
    sprintf(writeArtist, "%s - %s", artist, album);
    sprintf(writeDuration, "%i:%02i", musicDuration / 60, musicDuration % 60);
    sprintf(writeTime, "%i:%02i", displayMusicPosition / 60, displayMusicPosition % 60);
    sprintf(fileImageName, "%s.png", imageName);

    video_screenBlack();
    video_drawRectangle(185, 258, (int) ((double) displayMusicPosition * 422.0 / (double) musicDuration), 12, 255, 186,
                        0);
    video_screenAddImage(SYSTEM_RESOURCES, "musicPlayer.png", 0, 0, 640);
    video_screenAddImage(MUSICPLAYER_RESOURCES, fileImageName, 24, 176, 128);
    video_screenWriteFont(writeTitle, fontBold24, colorWhite, 185, 190, SDL_ALIGN_LEFT);
    video_screenWriteFont(writeArtist, fontRegular20, colorWhite, 185, 222, SDL_ALIGN_LEFT);
    video_screenWriteFont(writeTime, fontRegular18, colorWhite, 185, 275, SDL_ALIGN_LEFT);
    video_screenWriteFont(writeDuration, fontRegular18, colorWhite, 605, 275, SDL_ALIGN_RIGHT);
    if(!parameters_getMusicDisableRepeatModes()) {
        char imageRepeatMode[STR_MAX];
        sprintf(imageRepeatMode, "musicPlayerRepeatMode%i.png", musicPlayerRepeatMode);
        video_screenAddImage(SYSTEM_RESOURCES, imageRepeatMode, 332, 274, 128);
    }
    musicplayer_interfaceplayer_drawSideMusic(-2, 0);
    musicplayer_interfaceplayer_drawSideMusic(-1, 83);
    musicplayer_interfaceplayer_drawSideMusic(1, 314);
    musicplayer_interfaceplayer_drawSideMusic(2, 397);
    video_applyToVideo();
}


void musicplayer_interfacealbum_drawAlbum(int baseIndex, int pos) {
    int albumIndex = baseIndex + pos;

    if (albumIndex >= musicPlayerAlbumsCount) {
        return;
    }

    int mIndex = musicPlayerAlbumsIndex[albumIndex];
    int length = strlen(musicPlayerTracksList[mIndex]) - 4;
    char fileImageName[STR_MAX], imageName[STR_MAX - 4];
    strncpy(imageName, musicPlayerTracksList[mIndex], length);
    imageName[length] = '\0';
    sprintf(fileImageName, "%s.png", imageName);

    int x = 51 + (pos % 3) * 205;
    int y = 28 + (pos / 3) * 148;

    if (musicPlayerAlbumIndex == albumIndex) {
        video_drawRectangle(x - 5, y - 5, 138, 138, 255, 186, 0);
    }
    video_screenAddImage(MUSICPLAYER_RESOURCES, fileImageName, x, y, 128);
}

void musicplayer_interfacealbum_draw() {
    if (musicPlayerAlbumIndex >= musicPlayerAlbumsCount) {
        musicPlayerAlbumIndex = 0;
    } else if (musicPlayerAlbumIndex < 0) {
        musicPlayerAlbumIndex = musicPlayerAlbumsCount - 1;
    }

    int page = musicPlayerAlbumIndex / 9;
    int baseIndex = page * 9;
    char writePage[STR_MAX];
    sprintf(writePage, "%i / %i", musicPlayerAlbumIndex + 1, musicPlayerAlbumsCount);

    video_screenAddImage(SYSTEM_RESOURCES, "musicPlayerAlbums.png", 0, 0, 640);
    musicplayer_interfacealbum_drawAlbum(baseIndex, 0);
    musicplayer_interfacealbum_drawAlbum(baseIndex, 1);
    musicplayer_interfacealbum_drawAlbum(baseIndex, 2);
    musicplayer_interfacealbum_drawAlbum(baseIndex, 3);
    musicplayer_interfacealbum_drawAlbum(baseIndex, 4);
    musicplayer_interfacealbum_drawAlbum(baseIndex, 5);
    musicplayer_interfacealbum_drawAlbum(baseIndex, 6);
    musicplayer_interfacealbum_drawAlbum(baseIndex, 7);
    musicplayer_interfacealbum_drawAlbum(baseIndex, 8);
    video_screenWriteFont(writePage, fontRegular16, colorWhite60, 590, 456, SDL_ALIGN_RIGHT);
    video_applyToVideo();
}

int musicplayer_getCurrentAlbumIndex(void) {
    int index = musicPlayerAlbumsCount - 1;
    for (int i = 1; i < musicPlayerAlbumsCount; i++) {
        if (musicPlayerTrackIndex < musicPlayerAlbumsIndex[i]) {
            index = i - 1;
            break;
        }
    }
    return index;
}

int musicplayer_getTrackPosition(void) {
    int mPos = musicPlayerTrackPosition;
    if (Mix_PlayingMusic() == 1 && Mix_PausedMusic() != 1) {
        mPos += get_time() - musicPlayerTrackStartTime;
    }
    return mPos;
}

void musicplayer_screenUpdate(void) {
    if (!display_enabled || musicPlayerTracksList == NULL) {
        return;
    }

    long int ts = get_time();

    if (musicPlayerScreenUpdate != ts) {
        bool lockChanged = applock_isLockRecentlyChanged() || applock_isUnlocking();
        int inactivityTime = ts - musicPlayerLastActivity;

        if (inactivityTime > 10 && !lockChanged) {
            video_displayBlackScreen();
            display_setScreen(false);
            return;
        }

        if (musicPlayerMode == MUSICPLAYER_MODE_PLAYER) {
            musicplayer_interfaceplayer_drawInterface(musicplayer_getTrackPosition());
        }
    }
}

void musicplayer_screenActivate(void) {
    musicPlayerLastActivity = get_time();
    display_setScreen(true);
}

void musicplayer_setMode(int mode) {
    musicPlayerMode = mode;
    if (musicPlayerMode == MUSICPLAYER_MODE_PLAYER) {
        musicplayer_screenUpdate();
    } else {
        musicPlayerAlbumIndex = musicplayer_getCurrentAlbumIndex();
        musicplayer_interfacealbum_draw();
    }
}

void musicplayer_load(void) {
    if (musicPlayerTracksCount == 0) {
        video_displayImage(SYSTEM_RESOURCES, "noMusic.png");
        return;
    }

    musicplayer_loadSession();

    if (musicPlayerTrackIndex < 0) {
        musicPlayerTrackIndex = musicPlayerTracksCount - 1;
    } else if (musicPlayerTrackIndex >= musicPlayerTracksCount) {
        musicPlayerTrackIndex = 0;
    }

    musicPlayerTrackStartTime = get_time();
    audio_play(MUSICPLAYER_RESOURCES, musicPlayerTracksList[musicPlayerTrackIndex], (double) musicPlayerTrackPosition);
    Mix_HookMusicFinished(callback_musicplayer_autoplay);

    if (display_enabled && musicPlayerMode == MUSICPLAYER_MODE_PLAYER) {
        musicplayer_interfaceplayer_drawInterface(musicPlayerTrackPosition);
    }
}

void musicplayer_changeAlbum(int direction) {
    musicPlayerAlbumIndex += direction;
    musicplayer_interfacealbum_draw();
}

void musicplayer_changeSong(int direction) {
    if (musicPlayerTracksCount == 0) {
        return;
    }
    musicPlayerTrackPosition = 0;
    if (musicPlayerRepeatMode == MUSICPLAYER_REPEAT_ALL) {
        musicPlayerTrackIndex += direction;
    } else if (musicPlayerRepeatMode == MUSICPLAYER_REPEAT_SHUFFLE) {
        musicPlayerTrackIndex = rand() % musicPlayerTracksCount;
    } else if (musicPlayerRepeatMode == MUSICPLAYER_REPEAT_ALBUM) {
        int oldAlbumIndex = musicplayer_getCurrentAlbumIndex();
        musicPlayerTrackIndex += direction;
        if (musicPlayerTrackIndex < 0) {
            if(musicPlayerAlbumsCount > 1) {
                musicPlayerTrackIndex = musicPlayerAlbumsIndex[1] - 1;
            } else {
                musicPlayerTrackIndex = musicPlayerTracksCount - 1;
            }
        } else if (musicPlayerTrackIndex >= musicPlayerTracksCount) {
            musicPlayerTrackIndex = musicPlayerAlbumsIndex[musicPlayerAlbumsCount - 1];
        } else {
            int newAlbumIndex = musicplayer_getCurrentAlbumIndex();
            if (oldAlbumIndex > newAlbumIndex) {
                int nextAlbumIndex = oldAlbumIndex + 1;
                if(nextAlbumIndex >= musicPlayerAlbumsCount) {
                    musicPlayerTrackIndex = musicPlayerTracksCount - 1;
                } else {
                    musicPlayerTrackIndex = musicPlayerAlbumsIndex[nextAlbumIndex] - 1;
                }
            } else if (oldAlbumIndex < newAlbumIndex) {
                musicPlayerTrackIndex = musicPlayerAlbumsIndex[oldAlbumIndex];
            }
        }
    }

    musicplayer_load();
}

void musicplayer_nextSong(void) {
    musicplayer_changeSong(1);
}

void musicplayer_up(void) {
    if (musicPlayerTracksCount == 0) {
        return;
    }
    musicplayer_screenActivate();
    if (musicPlayerMode == MUSICPLAYER_MODE_PLAYER) {
        musicplayer_changeSong(-1);
    } else {
        musicplayer_changeAlbum(-3);
    }
}

void musicplayer_down(void) {
    if (musicPlayerTracksCount == 0) {
        return;
    }
    musicplayer_screenActivate();
    if (musicPlayerMode == MUSICPLAYER_MODE_PLAYER) {
        musicplayer_nextSong();
    } else {
        musicplayer_changeAlbum(3);
    }
}

void musicplayer_rewind(int time) {
    int musicDuration = audio_getDuration();
    long int ts = get_time();
    musicPlayerTrackPosition += ts - musicPlayerTrackStartTime + time;
    musicPlayerTrackStartTime = ts;
    if (musicPlayerTrackPosition < 0) {
        musicPlayerTrackPosition = 0;
    } else if (musicPlayerTrackPosition >= musicDuration) {
        musicPlayerTrackPosition = musicDuration - 1;
    }
    audio_setPosition((double) musicPlayerTrackPosition);
    musicplayer_interfaceplayer_drawInterface(musicPlayerTrackPosition);
}

void musicplayer_next(void) {
    if (musicPlayerTracksCount == 0) {
        return;
    }
    musicplayer_screenActivate();
    if (musicPlayerMode == MUSICPLAYER_MODE_PLAYER) {
        musicplayer_rewind(10);
    } else {
        musicplayer_changeAlbum(1);
    }
}

void musicplayer_previous(void) {
    if (musicPlayerTracksCount == 0) {
        return;
    }
    musicplayer_screenActivate();
    if (musicPlayerMode == MUSICPLAYER_MODE_PLAYER) {
        musicplayer_rewind(-10);
    } else {
        musicplayer_changeAlbum(-1);
    }
}

void musicplayer_ok(void) {
    if (musicPlayerTracksCount == 0) {
        return;
    }
    if(!display_enabled) {
        musicplayer_screenActivate();
        return;
    }
    if (musicPlayerMode == MUSICPLAYER_MODE_PLAYER) {
        if(!parameters_getMusicDisableRepeatModes()) {
            musicPlayerRepeatMode++;
            if (musicPlayerRepeatMode > MUSICPLAYER_REPEAT_TITLE) {
                musicPlayerRepeatMode = MUSICPLAYER_REPEAT_ALL;
            } else if (musicPlayerRepeatMode < MUSICPLAYER_REPEAT_ALL) {
                musicPlayerRepeatMode = MUSICPLAYER_REPEAT_TITLE;
            }
            musicplayer_interfaceplayer_drawInterface(musicplayer_getTrackPosition());
        }
    } else {
        musicPlayerMode = MUSICPLAYER_MODE_PLAYER;
        musicPlayerTrackPosition = 0;
        musicPlayerTrackIndex = musicPlayerAlbumsIndex[musicPlayerAlbumIndex];
        musicplayer_load();
    }
}


void musicplayer_pause(void) {
    if (musicPlayerTracksCount == 0) {
        return;
    }
    musicplayer_screenActivate();
    if (Mix_PlayingMusic() == 1) {
        if (Mix_PausedMusic() == 1) {
            Mix_ResumeMusic();
            musicPlayerTrackStartTime = get_time();
            audio_setPosition((double) musicPlayerTrackPosition);
            musicplayer_autosleep_lock();
        } else {
            Mix_PauseMusic();
            musicPlayerTrackPosition += get_time() - musicPlayerTrackStartTime;
            musicplayer_autosleep_unlock();
        }
    }
}

bool musicplayer_home(void) {
    if (musicPlayerTracksCount == 0) {
        return true;
    }
    if (musicPlayerMode == MUSICPLAYER_MODE_PLAYER) {
        if (Mix_PlayingMusic() == 1 && Mix_PausedMusic() != 1) {
            musicPlayerTrackPosition += get_time() - musicPlayerTrackStartTime;
        }
        audio_free_music();
        return true;
    } else {
        musicplayer_screenActivate();
        musicplayer_setMode(MUSICPLAYER_MODE_PLAYER);
        return false;
    }
}

void musicplayer_lockChanged(void) {
    bool lockChanged = applock_isLockRecentlyChanged() || applock_isUnlocking();
    if (lockChanged && !display_enabled) {
        display_setScreen(true);
    }
    musicplayer_setMode(musicPlayerMode);
}

void musicplayer_menu(void) {
    if (musicPlayerTracksCount == 0) {
        return;
    }

    musicplayer_screenActivate();
    if (musicPlayerMode == MUSICPLAYER_MODE_PLAYER) {
        musicplayer_setMode(MUSICPLAYER_MODE_ALBUM);
    } else {
        musicplayer_setMode(MUSICPLAYER_MODE_PLAYER);
    }
}

void musicplayer_save(void) {
    if (Mix_PlayingMusic() == 1) {
        if (Mix_PausedMusic() != 1) {
            musicplayer_pause();
        }
    }

    musicplayer_saveSession();
}

bool musicplayer_isMp3File(const char *fileName) {
    return strcmp((char *) fileName + strlen(fileName) - 4, ".mp3") == 0;
}

bool musicplayer_isNewAlbum(const char *fileName, char *lastAlbum) {
    char *pointer = strchr(fileName, '_');
    pointer = strchr(pointer + 1, '_');

    char albumName[STR_MAX];
    strncpy(albumName, fileName, pointer - fileName);
    albumName[pointer - fileName] = '\0';

    if (strcmp(albumName, lastAlbum) == 0) {
        return false;
    }

    strcpy(lastAlbum, albumName);
    return true;
}

void musicplayer_init(void) {
    musicplayer_autosleep_lock();
    musicplayer_screenActivate();

    if (musicPlayerTracksList != NULL) {
        return musicplayer_load();
    }

    callback_musicplayer_autoplay = &musicplayer_nextSong;

    video_displayImage(SYSTEM_RESOURCES, "loadingMusic.png");

    musicPlayerTracksCount = 0;
    DIR *d;
    struct dirent *dir;
    d = opendir(MUSICPLAYER_RESOURCES);

    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_REG && musicplayer_isMp3File(dir->d_name)) {
            musicPlayerTracksCount++;
        }
    }

    if (musicPlayerTracksCount > 0) {
        musicPlayerTracksList = (char **) malloc(musicPlayerTracksCount * sizeof(char *));
        int i = 0;

        rewinddir(d);
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG && musicplayer_isMp3File(dir->d_name)) {
                musicPlayerTracksList[i] = malloc(STR_MAX);
                strcpy(musicPlayerTracksList[i], dir->d_name);
                i++;
            }
        }

        sort(musicPlayerTracksList, musicPlayerTracksCount);

        char lastAlbum[STR_MAX] = {'\0'};
        musicPlayerAlbumsCount = 0;
        for (i = 0; i < musicPlayerTracksCount; i++) {
            if (musicplayer_isNewAlbum(musicPlayerTracksList[i], lastAlbum)) {
                musicPlayerAlbumsCount++;
            }
        }

        musicPlayerAlbumsIndex = (int *) malloc(sizeof(int) * musicPlayerAlbumsCount);
        lastAlbum[0] = '\0';
        int j = 0;
        for (i = 0; i < musicPlayerTracksCount; i++) {
            if (musicplayer_isNewAlbum(musicPlayerTracksList[i], lastAlbum)) {
                musicPlayerAlbumsIndex[j] = i;
                j++;
            }
        }
    }

    closedir(d);
    musicplayer_load();
}

#endif // STORYTELLER_MUSICPLAYER_HELPER__