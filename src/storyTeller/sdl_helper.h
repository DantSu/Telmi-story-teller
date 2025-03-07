#ifndef STORYTELLER_SDL_HELPER__
#define STORYTELLER_SDL_HELPER__

#include <math.h>

#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include "SDL2/SDL_gfx.h"

#include "./logs_helper.h"
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

static SDL_Window *window = NULL;
static SDL_Surface *screen = NULL;
static SDL_Surface *appSurface = NULL;
static SDL_Texture *texture = NULL;
static SDL_Renderer *renderer = NULL;
static Mix_Music *music;
static double musicDuration;
static TTF_Font *fontBold24;
static TTF_Font *fontBold20;
static TTF_Font *fontBold18;
static TTF_Font *fontRegular20;
static TTF_Font *fontRegular18;
static TTF_Font *fontRegular16;

static SDL_Color colorWhite = {255, 255, 255};
static SDL_Color colorWhite60 = {189, 186, 193};
static SDL_Color colorPurple = {37, 16, 58};
static SDL_Color colorOrange = {255, 181, 0};
static SDL_Color colorRed = {238, 45, 0};


static SDL_Surface *cacheSurfaces[16] = {NULL, NULL, NULL, NULL,
                                         NULL, NULL, NULL, NULL,
                                         NULL, NULL, NULL, NULL,
                                         NULL, NULL, NULL, NULL};
static char cacheSurfacesKeys[16][STR_MAX * 2 + 12] = {{'\0'},{'\0'},{'\0'},{'\0'},
                                                       {'\0'},{'\0'},{'\0'},{'\0'},
                                                       {'\0'},{'\0'},{'\0'},{'\0'},
                                                       {'\0'},{'\0'},{'\0'},{'\0'}};

SDL_Surface *video_findCacheSurface(char* surfaceKey) {
    for (int i = 0; i < 16; ++i) {
        if (strcmp(surfaceKey, cacheSurfacesKeys[i]) != 0) {
            continue;
        }

        SDL_Surface *tmpSurface = cacheSurfaces[i];
        for (int j = i; j > 0; --j) {
            strcpy(cacheSurfacesKeys[j], cacheSurfacesKeys[j - 1]);
            cacheSurfaces[j] = cacheSurfaces[j - 1];
        }
        strcpy(cacheSurfacesKeys[0], surfaceKey);
        cacheSurfaces[0] = tmpSurface;
        return tmpSurface;
    }
    return NULL;
}

void video_saveCacheSurface(char *surfaceKey, SDL_Surface *surface) {
    if (cacheSurfaces[15] != NULL) {
        SDL_FreeSurface(cacheSurfaces[15]);
    }
    for (int i = 15; i > 0; --i) {
        strcpy(cacheSurfacesKeys[i], cacheSurfacesKeys[i - 1]);
        cacheSurfaces[i] = cacheSurfaces[i - 1];
    }
    strcpy(cacheSurfacesKeys[0], surfaceKey);
    cacheSurfaces[0] = surface;
}

SDL_Surface *video_loadAndCacheImage(char *image_path) {
    SDL_Surface *image = video_findCacheSurface(image_path);
    if (image == NULL) {
        image = IMG_Load(image_path);
        if (image != NULL) {
            video_saveCacheSurface(image_path, image);
        }
    }
    return image;
}

void video_screenBlack(void) {
    SDL_FillRect(appSurface, NULL, 0);
}

void video_drawRectangle(int x, int y, int width, int height, Uint8 r, Uint8 g, Uint8 b) {
    SDL_FillRect(appSurface, &(SDL_Rect) {x, y, width, height}, SDL_MapRGB(appSurface->format, r, g, b));
}

void video_screenAddImage(const char *dir, char *name, int x, int y, int width) {
    char image_path[STR_MAX * 2];
    char image_key[STR_MAX * 2 + 12];
    sprintf(image_path, "%s%s", dir, name);
    sprintf(image_key, "%s|%i", image_path, width);

    SDL_Surface *image = video_findCacheSurface(image_key);

    if (image != NULL) {
        SDL_BlitSurface(image, NULL, appSurface, &(SDL_Rect) {x, y});
        return;
    }

    image = IMG_Load(image_path);

    if (image == NULL) {
        return;
    }

    if (width != image->w) {
        SDL_Surface *imageScaled = rotozoomSurface(image, 0.0, (double) width / (double) image->w, 1);
        if (imageScaled != NULL) {
            SDL_BlitSurface(imageScaled, NULL, appSurface, &(SDL_Rect) {x, y});
            video_saveCacheSurface(image_key, imageScaled);
        }
        SDL_FreeSurface(image);
    } else {
        SDL_BlitSurface(image, NULL, appSurface, &(SDL_Rect) {x, y});
        video_saveCacheSurface(image_key, image);
    }
}

void video_screenWriteFont(const char *text, TTF_Font *font, SDL_Color color, int x, int y, int align) {
    SDL_Surface *sdlText = TTF_RenderUTF8_Blended(font, text, color);
    if (sdlText != NULL) {
        SDL_BlitSurface(sdlText, NULL, appSurface, &(SDL_Rect) {x - (sdlText->w / align), y});
        SDL_FreeSurface(sdlText);
    }
}

void video_applyToVideo(void) {
    int batteryPercentage = battery_getPercentage();
    SDL_Color colorBattery;
    if (batteryPercentage < 6) {
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

    SDL_BlitSurface(appSurface, NULL, screen, NULL);

    if (applock_isLocked()) {
        char image_path[STR_MAX * 2];
        sprintf(image_path, "%s%s", SYSTEM_RESOURCES, "storytellerLock.png");
        SDL_Surface *image = video_loadAndCacheImage(image_path);
        SDL_BlitSurface(image, NULL, screen, NULL);
    } else if (applock_isRecentlyUnlocked()) {
        char image_path[STR_MAX * 2];
        sprintf(image_path, "%s%s", SYSTEM_RESOURCES, "storytellerUnlock.png");
        SDL_Surface *image = video_loadAndCacheImage(image_path);
        SDL_BlitSurface(image, NULL, screen, NULL);
    }

    SDL_RenderClear(renderer);
    SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void video_displayImage(const char *dir, char *name) {
    char image_path[STR_MAX * 2];
    sprintf(image_path, "%s%s", dir, name);

    SDL_Surface *image = video_loadAndCacheImage(image_path);

    SDL_FillRect(appSurface, NULL, 0);
    if (image != NULL) {
        SDL_BlitSurface(
                image,
                NULL,
                appSurface,
                &(SDL_Rect) {(appSurface->w - image->w) / 2, (appSurface->h - image->h) / 2}
        );
    }
    video_applyToVideo();
}

void video_displayBlackScreen(void) {
    video_screenBlack();
    video_applyToVideo();
}

void audio_free_music(void) {
    if (music != NULL) {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
        music = NULL;
    }
}

void audio_setPosition(double position) {
    if (music != NULL && Mix_PlayingMusic() == 1) {
        Mix_SetMusicPosition(position);
    }
}

double audio_getDuration(void) {
    return musicDuration;
}

double audio_getPosition(void) {
    if (music != NULL) {
        return Mix_GetMusicPosition(music);
    }
    return 0.0;
}

void audio_play_path(char *sound_path, double position) {
    audio_free_music();
    music = Mix_LoadMUS(sound_path);
    if (music != NULL) {
        musicDuration = Mix_MusicDuration(music);
        Mix_PlayMusic(music, 1);
        Mix_SetMusicPosition(position);
    } else {
        musicDuration = 0.0;
    }
}

void audio_play(const char *dir, const char *name, double position) {
    char sound_path[STR_MAX * 2];
    sprintf(sound_path, "%s%s", dir, name);
    audio_play_path(sound_path, position);
}

void video_audio_init(void) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_Init(MIX_INIT_MP3);
    Mix_Volume(-1, MIX_MAX_VOLUME);
    Mix_VolumeMusic(MIX_MAX_VOLUME);

    window = SDL_CreateWindow("main", 0, 0, 640, 480, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    screen = SDL_CreateRGBSurface(0, 640, 480, 32, 0, 0, 0, 0);
    appSurface = SDL_CreateRGBSurface(0, screen->w, screen->h, 32, 0, 0, 0, 0);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, screen->w, screen->h);

    fontBold24 = TTF_OpenFont(FALLBACK_FONT_BOLD, 24);
    fontBold20 = TTF_OpenFont(FALLBACK_FONT_BOLD, 20);
    fontBold18 = TTF_OpenFont(FALLBACK_FONT_BOLD, 18);
    fontRegular20 = TTF_OpenFont(FALLBACK_FONT_REGULAR, 20);
    fontRegular18 = TTF_OpenFont(FALLBACK_FONT_REGULAR, 18);
    fontRegular16 = TTF_OpenFont(FALLBACK_FONT_REGULAR, 16);
}


void video_audio_quit(void) {
    TTF_Quit();

    if (music != NULL) {
        Mix_FreeMusic(music);
        music = NULL;
    }
    Mix_CloseAudio();

    SDL_FreeSurface(appSurface);
    SDL_FreeSurface(screen);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

#endif // STORYTELLER_SDL_HELPER__