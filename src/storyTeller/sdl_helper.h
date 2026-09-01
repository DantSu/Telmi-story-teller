#ifndef STORYTELLER_SDL_HELPER__
#define STORYTELLER_SDL_HELPER__

#include <math.h>
#include <pthread.h>
#include <stdint.h>

#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include "SDL2/SDL_gfx.h"

#include "system/display.h"
#include "utils/str.h"

#include "./logs_helper.h"
#include "./app_battery.h"
#include "./app_lock.h"
#include "./app_parameters.h"
#include "./app_volume.h"
#include "./app_brightness.h"

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

#define SURFACE_CACHE_SIZE 32

typedef struct {
    uint64_t hash;
    SDL_Surface *surface;
} surfaceCacheEntry;

static surfaceCacheEntry surfaceCache[SURFACE_CACHE_SIZE];


static Mix_Music *music;
static double musicDuration;
static pthread_mutex_t durationThreadMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t durationThread;
static bool killDurationThread = false;
static char durationThreadPath[STR_MAX * 2];
static char currentMusicPath[STR_MAX * 2];

#define AUDIO_DURATION_CACHE_SIZE 128

typedef struct {
    uint64_t hash;
    double duration;
} audioDurationCacheEntry;

static audioDurationCacheEntry audioDurationCache[AUDIO_DURATION_CACHE_SIZE];

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


static uint64_t string_hash(const char *path) {
    uint64_t h = 0xcbf29ce484222325ULL;
    while (*path != '\0') {
        h ^= (uint8_t) *path;
        h *= 0x100000001b3ULL;
        path++;
    }
    return h;
}

static uint64_t video_surfaceCacheHash(const char *path, int width) {
    uint64_t hash = string_hash(path);
    uint32_t w = (uint32_t) width;
    for (int i = 0; i < 4; ++i) {
        hash ^= (uint8_t) ((w >> (8 * i)) & 0xff);
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

SDL_Surface *video_findCacheSurface(uint64_t hash, const char *path) {
    int i = 0;
    while (i < SURFACE_CACHE_SIZE && surfaceCache[i].hash != hash) {
        ++i;
    }
    SDL_Surface *surface = NULL;
    if (i < SURFACE_CACHE_SIZE) {
        surfaceCacheEntry entry = surfaceCache[i];
        memmove(&surfaceCache[1], &surfaceCache[0], i * sizeof(surfaceCache[0]));
        surfaceCache[0] = entry;
        surface = entry.surface;
    }
    return surface;
}

void video_saveCacheSurface(uint64_t hash, const char *path, SDL_Surface *surface) {
    if (surfaceCache[SURFACE_CACHE_SIZE - 1].surface != NULL) {
        SDL_FreeSurface(surfaceCache[SURFACE_CACHE_SIZE - 1].surface);
    }
    memmove(&surfaceCache[1], &surfaceCache[0], (SURFACE_CACHE_SIZE - 1) * sizeof(surfaceCache[0]));
    surfaceCache[0].hash = hash;
    surfaceCache[0].surface = surface;
}

SDL_Surface *video_loadAndCacheImage(char *imagePath) {
    uint64_t hash = string_hash(imagePath);
    SDL_Surface *image = video_findCacheSurface(hash, imagePath);
    if (image == NULL) {
        image = IMG_Load(imagePath);
        if (image != NULL) {
            video_saveCacheSurface(hash, imagePath, image);
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
    char imagePath[STR_MAX * 2];
    sprintf(imagePath, "%s%s", dir, name);
    uint64_t hash = video_surfaceCacheHash(imagePath, width);

    SDL_Surface *image = video_findCacheSurface(hash, imagePath);

    if (image != NULL) {
        SDL_BlitSurface(image, NULL, appSurface, &(SDL_Rect) {x, y});
        return;
    }

    image = IMG_Load(imagePath);

    if (image == NULL) {
        return;
    }

    if (width != image->w) {
        SDL_Surface *imageScaled = rotozoomSurface(image, 0.0, (double) width / (double) image->w, 1);
        if (imageScaled != NULL) {
            SDL_BlitSurface(imageScaled, NULL, appSurface, &(SDL_Rect) {x, y});
            video_saveCacheSurface(hash, imagePath, imageScaled);
        }
        SDL_FreeSurface(image);
    } else {
        SDL_BlitSurface(image, NULL, appSurface, &(SDL_Rect) {x, y});
        video_saveCacheSurface(hash, imagePath, image);
    }
}

void video_screenWriteFont(const char *text, TTF_Font *font, SDL_Color color, int x, int y, int align) {
    SDL_Surface *sdlText = TTF_RenderUTF8_Blended(font, text, color);
    if (sdlText != NULL) {
        SDL_BlitSurface(sdlText, NULL, appSurface, &(SDL_Rect) {x - (sdlText->w / align), y});
        SDL_FreeSurface(sdlText);
    }
}

void video_showBattery(void) {
    int batteryPercentage = app_battery_getPercentage();
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
}

void video_showRam(void) {
    long totalKb = 0;
    long availableKb = 0;
    FILE *file = fopen("/proc/meminfo", "r");
    if (file != NULL) {
        char line[128];
        while (fgets(line, sizeof(line), file) != NULL) {
            if (strncmp(line, "MemTotal:", 9) == 0) {
                sscanf(line, "MemTotal: %ld kB", &totalKb);
            } else if (strncmp(line, "MemAvailable:", 13) == 0) {
                sscanf(line, "MemAvailable: %ld kB", &availableKb);
            }
        }
        fclose(file);
    }
    long usedKb = totalKb - availableKb;

    char ramText[64];
    sprintf(ramText, "RAM : %ld Mo utilisés, %ld Mo libres, %ld Mo total", usedKb / 1024, availableKb / 1024, totalKb / 1024);
    video_screenWriteFont(ramText, fontRegular16, colorWhite60, 380, 2, SDL_ALIGN_RIGHT);
}

void video_showBar(void) {
    int height, heightMax;
    char imageName[32];
    if(app_brightness_isShowed()) {
        height = app_brightness_getCurrent() * 350 / parameters_getSystemScreenBrightnessMax();
        heightMax = parameters_getScreenBrightnessMax() * 350 / parameters_getSystemScreenBrightnessMax();
        sprintf(imageName, "%s", "storytellerBrightnessBar.png");
    } else if (app_volume_isShowed()) {
        height = app_volume_getCurrent() * 350 / parameters_getSystemAudioVolumeMax();
        heightMax = parameters_getAudioVolumeMax() * 350 / parameters_getSystemAudioVolumeMax();
        sprintf(imageName, "%s", "storytellerVolumeBar.png");
    } else {
        return;
    }

    SDL_FillRect(screen, &(SDL_Rect) {19, 47, 26, 350}, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_FillRect(screen, &(SDL_Rect) {19, 397 - height, 26, height}, SDL_MapRGB(screen->format, 255, 186, 0));
    if(heightMax < 350) {
        SDL_FillRect(screen, &(SDL_Rect) {19, 397 - heightMax, 26, 2}, SDL_MapRGB(screen->format, 238, 45, 0));
    }

    char imagePath[STR_MAX * 2];
    sprintf(imagePath, "%s%s", SYSTEM_RESOURCES, imageName);
    SDL_Surface *image = video_loadAndCacheImage(imagePath);
    SDL_BlitSurface(image, NULL, screen, NULL);
}

void video_showAppLock(void) {
    if (!applock_isLocked() && !applock_isRecentlyUnlocked()) {
        return;
    }
    char imagePath[STR_MAX * 2];
    sprintf(imagePath, "%s%s", SYSTEM_RESOURCES, applock_isLocked() ? "storytellerLock.png" : "storytellerUnlock.png");
    SDL_Surface *image = video_loadAndCacheImage(imagePath);
    SDL_BlitSurface(image, NULL, screen, NULL);
}

void video_applyToVideo(void) {
    video_showBattery();
    video_showRam();
    SDL_BlitSurface(appSurface, NULL, screen, NULL);
    video_showAppLock();
    video_showBar();

    SDL_RenderClear(renderer);
    SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void video_displayImage(const char *dir, char *name) {
    char imagePath[STR_MAX * 2];
    sprintf(imagePath, "%s%s", dir, name);

    SDL_Surface *image = video_loadAndCacheImage(imagePath);

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

double audio_duration_cache_get(const char *path) {
    uint64_t hash = string_hash(path);
    pthread_mutex_lock(&durationThreadMutex);
    double duration = -1.0;
    int i = 0;
    while (i < AUDIO_DURATION_CACHE_SIZE && audioDurationCache[i].hash != hash) {
        ++i;
    }
    if (i < AUDIO_DURATION_CACHE_SIZE) {
        audioDurationCacheEntry entry = audioDurationCache[i];
        memmove(&audioDurationCache[1], &audioDurationCache[0], i * sizeof(audioDurationCache[0]));
        audioDurationCache[0] = entry;
        duration = entry.duration;
    }
    pthread_mutex_unlock(&durationThreadMutex);
    return duration;
}

void audio_duration_cache_set(const char *path, double duration) {
    if (duration <= 0.0) {
        return;
    }
    uint64_t hash = string_hash(path);
    pthread_mutex_lock(&durationThreadMutex);
    memmove(&audioDurationCache[1], &audioDurationCache[0], (AUDIO_DURATION_CACHE_SIZE - 1) * sizeof(audioDurationCache[0]));
    audioDurationCache[0].hash = hash;
    audioDurationCache[0].duration = duration;
    pthread_mutex_unlock(&durationThreadMutex);
}

void *audio_calculate_duration_thread(void *arg) {
    char pathToCalculate[STR_MAX * 2];
    while (true) {

        pthread_mutex_lock(&durationThreadMutex);
        if (killDurationThread) {
            pthread_mutex_unlock(&durationThreadMutex);
            break;
        }

        if (durationThreadPath[0] == '\0') {
            pthread_mutex_unlock(&durationThreadMutex);
            sleep(1);
            continue;
        }

        strcpy(pathToCalculate, durationThreadPath);
        durationThreadPath[0] = '\0';
        pthread_mutex_unlock(&durationThreadMutex);

        Mix_Music *tempMusic = Mix_LoadMUS(pathToCalculate);
        if (tempMusic != NULL) {
            double duration = Mix_MusicDuration(tempMusic);
            Mix_FreeMusic(tempMusic);

            audio_duration_cache_set(pathToCalculate, duration);

            pthread_mutex_lock(&durationThreadMutex);
            if (strcmp(pathToCalculate, currentMusicPath) == 0) {
                musicDuration = duration;
            }
            pthread_mutex_unlock(&durationThreadMutex);
        }
    }
    return NULL;
}

bool audio_isFinished(void) {
    return music == NULL || Mix_PlayingMusic() == 0;
}

void audio_free_music(void) {
    if (music != NULL) {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
        music = NULL;
    }
    pthread_mutex_lock(&durationThreadMutex);
    durationThreadPath[0] = '\0';
    currentMusicPath[0] = '\0';
    musicDuration = 0.0;
    pthread_mutex_unlock(&durationThreadMutex);
}

void audio_setPosition(double position) {
    if (!audio_isFinished()) {
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

void audio_play_path(char *soundPath, double position, bool askDuration) {
    audio_free_music();
    music = Mix_LoadMUS(soundPath);
    if (music != NULL) {
        pthread_mutex_lock(&durationThreadMutex);
        musicDuration = -1.0;
        strcpy(currentMusicPath, soundPath);
        pthread_mutex_unlock(&durationThreadMutex);

        Mix_PlayMusic(music, 1);
        Mix_SetMusicPosition(position);

        if (askDuration) {
            double cachedDuration = audio_duration_cache_get(soundPath);
            if (cachedDuration >= 0.0) {
                pthread_mutex_lock(&durationThreadMutex);
                musicDuration = cachedDuration;
                pthread_mutex_unlock(&durationThreadMutex);
            } else {
                pthread_mutex_lock(&durationThreadMutex);
                strcpy(durationThreadPath, soundPath);
                pthread_mutex_unlock(&durationThreadMutex);
            }
        }
    }
}

void audio_play(const char *dir, const char *name, double position, bool askDuration) {
    char soundPath[STR_MAX * 2];
    sprintf(soundPath, "%s%s", dir, name);
    audio_play_path(soundPath, position, askDuration);
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

    window = SDL_CreateWindow("main", 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, SDL_WINDOW_SHOWN);
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

    currentMusicPath[0] = '\0';
    durationThreadPath[0] = '\0';
    pthread_create(&durationThread, NULL, audio_calculate_duration_thread, NULL);
}


void video_audio_quit(void) {
    pthread_mutex_lock(&durationThreadMutex);
    killDurationThread = true;
    pthread_mutex_unlock(&durationThreadMutex);
    pthread_join(durationThread, NULL);

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