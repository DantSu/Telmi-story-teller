#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils/file.h"
#include "utils/flags.h"
#include "utils/log.h"

#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"
#define FALLBACK_FONT "/customer/app/Exo-2-Bold-Italic.ttf"

SDL_Surface *loadImage(const char *name)
{
    char image_path[512];
    sprintf(image_path, "%s%s.png", SYSTEM_RESOURCES, name);
    return IMG_Load(image_path);
}

TTF_Font *loadFont(int size)
{
    return TTF_OpenFont(FALLBACK_FONT, size);
}

int main(int argc, char *argv[])
{
    // Boot : Loading screen
    // End_Save : Ending screen with save
    // End : Ending screen without save

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("main", 0, 0, 640, 480, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Surface *screen = SDL_CreateRGBSurface(0, 640, 480, 32, 0, 0, 0, 0);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, screen->w, screen->h);

    SDL_Surface *background;
    bool show_version = true;

    if (argc > 1 && strcmp(argv[1], "End") == 0) {
        background = loadImage("Screen_Off");
    } else if (argc > 1 && strcmp(argv[1], "lowBat") == 0) {
        background = loadImage("lowBat");
        show_version = false;
    } else {
        background = loadImage("bootScreen");
    }

    char message_str[STR_MAX] = "";
    if (argc > 2) {
        strncpy(message_str, argv[2], STR_MAX - 1);
    }

    if (background) {
        SDL_BlitSurface(background, NULL, screen, NULL);
        SDL_FreeSurface(background);
    }

    TTF_Font *font = loadFont(18);
    SDL_Color color = {255, 255, 255};

    if (show_version) {
        const char *version_str = file_read("/mnt/SDCARD/.tmp_update/onionVersion/version.txt");
        if (strlen(version_str) > 0) {
            SDL_Surface *version = TTF_RenderUTF8_Blended(font, version_str, color);
            if (version) {
                SDL_BlitSurface(version, NULL, screen, &(SDL_Rect){20, 450 - version->h / 2});
                SDL_FreeSurface(version);
            }
        }
    }

    if (strlen(message_str) > 0) {
        SDL_Surface *message = TTF_RenderUTF8_Blended(font, message_str, color);
        if (message) {
            SDL_BlitSurface(message, NULL, screen, &(SDL_Rect){620 - message->w, 450 - message->h / 2});
            SDL_FreeSurface(message);
        }
    }

    SDL_RenderClear(renderer);
    SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    SDL_Delay(500);

    if (argc > 1 && strcmp(argv[1], "Boot") != 0)
        temp_flag_set(".offOrder", false);

#ifndef PLATFORM_MIYOOMINI
    sleep(4); // for debugging purposes
#endif

    TTF_Quit();
    SDL_FreeSurface(screen);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
