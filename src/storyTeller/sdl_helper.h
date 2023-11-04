#ifndef STORYTELLER_SDL_HELPER__
#define STORYTELLER_SDL_HELPER__

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include "utils/str.h"

#define FALLBACK_FONT "/customer/app/Exo-2-Bold-Italic.ttf"

static SDL_Surface *video;
static SDL_Surface *screen;
static Mix_Music *music;

static TTF_Font *font;
static SDL_Color color = {255, 255, 255};
static int fontPosition = 0;

void video_audio_init(void) 
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();
    if(Mix_OpenAudio(48000, 32784, 2, 4096) < 0) {
        Mix_Volume(-1, MIX_MAX_VOLUME);
        Mix_VolumeMusic(MIX_MAX_VOLUME);
    }
    video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
    font = TTF_OpenFont(FALLBACK_FONT, 18);
}


void video_audio_quit(void) 
{
    TTF_quit();

    Mix_FreeMusic(music);
    Mix_CloseAudio();

    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    SDL_Quit();
}

void video_displayImage(const char *dir, const char *name)
{
    char image_path[STR_MAX * 2];
    sprintf(image_path, "%s%s", dir, name);
    SDL_Surface *image = IMG_Load(image_path);

    SDL_FillRect(screen, NULL, 0);
    SDL_BlitSurface(image, NULL, screen, &(SDL_Rect){(screen->w - image->w) / 2, (screen->h - image->h) / 2});
    SDL_FreeSurface(image);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
}


void audio_play(const char *dir, const char *name) {
    if(music != NULL) {
        Mix_FreeMusic(music);
    }
    char sound_path[STR_MAX * 2];
    sprintf(sound_path, "%s%s", dir, name);
    music = Mix_LoadMUS(sound_path);
    Mix_PlayMusic(music, 1);
}


void font_write(const char *text) 
{
    SDL_Surface *sdlText = TTF_RenderUTF8_Blended(font, text, color);
    if (sdlText) {
        fontPosition += 20;
        SDL_BlitSurface(sdlText, NULL, screen, &(SDL_Rect){20, fontPosition});
        SDL_FreeSurface(sdlText);
        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
    }
}


#endif // STORYTELLER_SDL_HELPER__