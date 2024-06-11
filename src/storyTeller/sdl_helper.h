#ifndef STORYTELLER_SDL_HELPER__
#define STORYTELLER_SDL_HELPER__

#include <math.h>

#include "SDL/SDL_rotozoom.h"
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include "system/battery.h"
#include "utils/str.h"
#include "./file_helper.h"
#include "./app_lock.h"

#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"

#define FALLBACK_FONT_REGULAR "/mnt/SDCARD/.tmp_update/res/Exo2-Regular.ttf"
#define FALLBACK_FONT_BOLD "/mnt/SDCARD/.tmp_update/res/Exo2-Bold.ttf"

#define SDL_ALIGN_LEFT 0
#define SDL_ALIGN_RIGHT 1
#define SDL_ALIGN_CENTER 2

static SDL_Surface *video;
static SDL_Surface *screen;
static Mix_Music *music;
static int musicDuration;
static TTF_Font *fontBold24;
static TTF_Font *fontBold20;
static TTF_Font *fontRegular20;
static TTF_Font *fontRegular18;
static TTF_Font *fontRegular16;

static SDL_Color colorWhite = {255, 255, 255};
static SDL_Color colorWhite60 = {189, 186, 193};
static SDL_Color colorPurple = {37, 16, 58};
static SDL_Color colorOrange = {255, 181, 0};
static SDL_Color colorRed = {238, 45, 0};


void video_audio_init(void) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    if (Mix_OpenAudio(44100, 32784, 2, 2048) < 0) {
        Mix_Volume(-1, MIX_MAX_VOLUME);
        Mix_VolumeMusic(MIX_MAX_VOLUME);
    }
    video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
    fontBold24 = TTF_OpenFont(FALLBACK_FONT_BOLD, 24);
    fontBold20 = TTF_OpenFont(FALLBACK_FONT_BOLD, 20);
    fontRegular20 = TTF_OpenFont(FALLBACK_FONT_REGULAR, 20);
    fontRegular18 = TTF_OpenFont(FALLBACK_FONT_REGULAR, 18);
    fontRegular16 = TTF_OpenFont(FALLBACK_FONT_REGULAR, 16);
}


void video_audio_quit(void) {
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    TTF_Quit();

    if (music != NULL) {
        Mix_FreeMusic(music);
        music = NULL;
    }
    Mix_CloseAudio();

    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    SDL_Quit();
}

void video_screenBlack(void) {
    SDL_FillRect(screen, NULL, 0);
}

void video_drawRectangle(int x, int y, int width, int height, Uint8 r, Uint8 g, Uint8 b) {
    SDL_FillRect(screen, &(SDL_Rect) {x, y, width, height}, SDL_MapRGB(screen->format, r, g, b));
}

void video_screenAddImage(const char *dir, char *name, int x, int y, int width) {
    char image_path[STR_MAX * 2];
    sprintf(image_path, "%s%s", dir, name);
    SDL_Surface *image = IMG_Load(image_path);

    if (image == NULL) {
        return;
    }

    if (width != image->w) {
        SDL_Surface *imageScaled = rotozoomSurface(image, 0.0, (double) width / (double) image->w, 0);
        if (imageScaled != NULL) {
            SDL_BlitSurface(imageScaled, NULL, screen, &(SDL_Rect) {x, y});
            SDL_FreeSurface(imageScaled);
        }
    } else {
        SDL_BlitSurface(image, NULL, screen, &(SDL_Rect) {x, y});
    }
    SDL_FreeSurface(image);
}

void video_screenWriteFont(const char *text, TTF_Font *font, SDL_Color color, int x, int y, int align) {
    SDL_Surface *sdlText = TTF_RenderUTF8_Blended(font, text, color);
    if (sdlText != NULL) {
        SDL_BlitSurface(sdlText, NULL, screen, &(SDL_Rect) {x - (sdlText->w / align), y});
        SDL_FreeSurface(sdlText);
    }
}

void video_applyToVideo(void) {
    int batteryPercentage = battery_getPercentage();
    SDL_Color colorBattery;
    if(batteryPercentage < 6) {
        colorBattery = colorRed;
        video_screenAddImage(SYSTEM_RESOURCES, "storytellerBatteryEmpty.png", 531, 2, 76);
    } else if (batteryPercentage < 20) {
        colorBattery = colorOrange;
        video_screenAddImage(SYSTEM_RESOURCES, "storytellerBatteryLow.png", 531, 2, 76);
    } else if (batteryPercentage < 60) {
        colorBattery = colorWhite60;
        video_screenAddImage(SYSTEM_RESOURCES, "storytellerBatteryMedium.png", 531, 2, 76);
    } else {
        colorBattery = colorWhite60;
        video_screenAddImage(SYSTEM_RESOURCES, "storytellerBatteryFull.png", 531, 2, 76);
    }

    char strBatteryPercent[6];
    sprintf(strBatteryPercent, "%i%%", batteryPercentage);
    video_screenWriteFont(strBatteryPercent, fontRegular16, colorBattery, 555, 2, SDL_ALIGN_CENTER);

    SDL_BlitSurface(screen, NULL, video, NULL);

    if (applock_isLocked()) {
        char image_path[STR_MAX * 2];
        sprintf(image_path, "%s%s", SYSTEM_RESOURCES, "storytellerLock.png");
        SDL_Surface *image = IMG_Load(image_path);
        SDL_BlitSurface(image, NULL, video, NULL);
    } else if (applock_isRecentlyUnlocked()) {
        char image_path[STR_MAX * 2];
        sprintf(image_path, "%s%s", SYSTEM_RESOURCES, "storytellerUnlock.png");
        SDL_Surface *image = IMG_Load(image_path);
        SDL_BlitSurface(image, NULL, video, NULL);
    }

    SDL_Flip(video);
}

void video_displayImage(const char *dir, char *name) {
    char image_path[STR_MAX * 2];
    sprintf(image_path, "%s%s", dir, name);
    SDL_Surface *image = IMG_Load(image_path);

    SDL_FillRect(screen, NULL, 0);
    if (image != NULL) {
        SDL_BlitSurface(image, NULL, screen, &(SDL_Rect) {(screen->w - image->w) / 2, (screen->h - image->h) / 2});
        SDL_FreeSurface(image);
    }

    video_applyToVideo();
}

void video_displayBlackScreen(void) {
    video_screenBlack();
    video_applyToVideo();
}

void audio_free_music(void) {
    if (music != NULL) {
        Mix_HookMusicFinished(NULL);
        Mix_HaltMusic();
        Mix_FreeMusic(music);
        music = NULL;
    }
}

void audio_setPosition(double position) {
    if (music != NULL && Mix_PlayingMusic() == 1) {
        Mix_RewindMusic();
        Mix_SetMusicPosition(position);
    }
}

int audio_getDuration(void) {
    return musicDuration;
}

void audio_play_path(char *sound_path, double position) {
    audio_free_music();
    musicDuration = (int) (((double) (file_getSize(sound_path) - 16300L) / 24000.0) + 0.5);
    music = Mix_LoadMUS(sound_path);
    if (music != NULL) {
        Mix_PlayMusic(music, 1);
        Mix_SetMusicPosition(position);
    }
}

void audio_play(const char *dir, const char *name, double position) {
    char sound_path[STR_MAX * 2];
    sprintf(sound_path, "%s%s", dir, name);
    audio_play_path(sound_path, position);
}

#endif // STORYTELLER_SDL_HELPER__