#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "system/osd.h"
#include "system/system.h"
#include "system/keymap_hw.h"
#include "system/settings.h"
#include "system/settings_sync.h"
#include "utils/file.h"
#include "utils/json.h"
#include "utils/log.h"
#include "utils/str.h"



#define FALLBACK_FONT "/customer/app/Exo-2-Bold-Italic.ttf"
#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"
#define STORIES_RESOURCES "/mnt/SDCARD/Stories/"


static SDL_Surface *video;
static SDL_Surface *screen;
static Mix_Music *music;

void video_audio_init(void) 
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    if(Mix_OpenAudio(48000, 32784, 2, 4096) < 0) {
        Mix_Volume(-1, MIX_MAX_VOLUME);
        Mix_VolumeMusic(MIX_MAX_VOLUME);
    }
    video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
}

void video_displayImage(const char *dir, const char *name)
{
    char image_path[STR_MAX];
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
    char sound_path[STR_MAX];
    sprintf(sound_path, "%s%s", dir, name);
    music = Mix_LoadMUS(sound_path);
    Mix_PlayMusic(music, 1);
}





static TTF_Font *font;
static SDL_Color color = {255, 255, 255};
static int fontPosition = 0;

void font_init(int size)
{
    TTF_Init();
    font = TTF_OpenFont(FALLBACK_FONT, size);
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





static char **storiesList = NULL;
static int storiesCount = 0;
static int storyIndex;

void stories_show(void)
{
    if(storyIndex < 0) {
        storyIndex = storiesCount - 1;
    } else if (storyIndex >= storiesCount) {
        storyIndex = 0;
    }
    
    char story_path[STR_MAX];
    sprintf(story_path, "%s%s/story.json", STORIES_RESOURCES, storiesList[storyIndex]);

    const char *json_str = NULL;
    if (!(json_str = file_read(story_path))) {
        return;
    }

    cJSON *root = cJSON_Parse(json_str);
    if(root == NULL) {
        return;
    }
    
    cJSON *stageNodes = cJSON_GetObjectItem(root, "stageNodes");
    if(stageNodes == NULL) {
        return;
    }

    cJSON *firstStageNode = cJSON_GetArrayItem(stageNodes, 0);
    if(firstStageNode == NULL) {
        return;
    }

    sprintf(story_path, "%s%s/assets/", STORIES_RESOURCES, storiesList[storyIndex]);
    char imageFilename[STR_MAX];

    if(json_getString(firstStageNode, "image", imageFilename)) {
        video_displayImage(story_path, imageFilename);
    }
    char soundFilename[STR_MAX];
    if(json_getString(firstStageNode, "audio", soundFilename)) {
        audio_play(story_path, soundFilename);
    }
}

void stories_next(void)
{
    storyIndex += 1;
    stories_show();
}

void stories_previous(void)
{
    storyIndex -= 1;
    stories_show();
}

void stories_init(void)
{
    int i=0;
    DIR *d;
    struct dirent *dir;
    d = opendir(STORIES_RESOURCES);

    while((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            i++;
        }
    }
    rewinddir(d);

    storiesCount = i;
    char **filesList = (char**)malloc(i * sizeof(char*));
    i=0;

    while((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            filesList[i] = (char*) malloc(64 * sizeof(char));
            strcpy(filesList[i], dir->d_name);
            i++;
        }
    }
    closedir(d);
    storiesList = filesList;
    stories_show();
}




// for ev.value
#define RELEASED 0
#define PRESSED 1
#define REPEAT 2

// Global Variables
static int input_fd;
static struct input_event ev;
static struct pollfd fds[1];

bool keyinput_isValid(void)
{
    read(input_fd, &ev, sizeof(ev));

    if (ev.type != EV_KEY || ev.value > REPEAT) {
        return false;
    }

    return true;
}



int main(int argc, char *argv[])
{
    font_init(18);

    video_audio_init();
    video_displayImage(SYSTEM_RESOURCES, "storiesLoading.png");

    settings_init();
    display_init();
    setVolume(settings.volume);
    display_setBrightness(settings.brightness);

    stories_init();

    // Prepare for Poll button input
    input_fd = open("/dev/input/event0", O_RDONLY);
    memset(&fds, 0, sizeof(fds));
    fds[0].fd = input_fd;
    fds[0].events = POLLIN;

    bool is_menu_pressed = false;

    while (1) {
        if (poll(fds, 1, 0) > 0) {
            if (!keyinput_isValid()) {
                continue;
            }

            switch (ev.value) {
                case PRESSED:
                    if(ev.code == HW_BTN_MENU) {
                        is_menu_pressed = true;
                    }
                    break;
                
                case RELEASED:
                    switch (ev.code)
                    {
                        case HW_BTN_POWER :
                            goto exit_loop;
                            break;
                        case HW_BTN_MENU :
                            is_menu_pressed = false;
                            break;
                        case HW_BTN_LEFT :
                            stories_previous();
                            break;
                        case HW_BTN_RIGHT :
                            stories_next();
                            break;
                        case HW_BTN_A :
                            
                            break;
                        case HW_BTN_B :
                            
                            break;
                        case HW_BTN_VOLUME_DOWN :
                            if(is_menu_pressed) {
                                settings_setBrightness(settings.brightness - 1, true, false);
                                osd_showBrightnessBar(settings.brightness);
                            } else {
                                settings_setVolume(settings.volume - 1, true);
                                osd_showVolumeBar(settings.volume, false);
                            }
                            break;
                        case HW_BTN_VOLUME_UP :
                            if(is_menu_pressed) {
                                settings_setBrightness(settings.brightness + 1, true, false);
                                osd_showBrightnessBar(settings.brightness);
                            } else {
                                settings_setVolume(settings.volume + 1, true);
                                osd_showVolumeBar(settings.volume, false);
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                
                default:
                    break;
            }
        }
    }
    
    exit_loop:

    Mix_FreeMusic(music);
    Mix_CloseAudio();

    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    SDL_Quit();

    system_shutdown();

    return EXIT_SUCCESS;
}
