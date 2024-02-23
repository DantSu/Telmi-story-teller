#ifndef STORYTELLER_STORIES_HELPER__
#define STORYTELLER_STORIES_HELPER__

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "time.h"
#include "system/display.h"
#include "utils/str.h"
#include "utils/json.h"

#include "./app_file.h"
#include "./app_autosleep.h"
#include "./sdl_helper.h"
#include "./app_parameters.h"
#include "./array_helper.h"
#include "./time_helper.h"

#define STR_DIRNAME 64

static char **storiesList = NULL;
static cJSON *storyJson = NULL;
static int storiesCount = 0;
static int storyIndex = 0;
static char storyStageKey[STR_MAX] = {'\0'};
static char storyActionKey[STR_MAX] = {'\0'};
static int storyActionOptionIndex = 0;
static int storyActionOptionsCount = 0;
static double storyTime = 0;
static long int storyStartTime = 0;
static bool storyAutoplay = false;
static bool storyOkAction = true;
static void (*callback_stories_autoplay)(void);
static void (*callback_stories_reset)(void);

#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"
#define STORIES_RESOURCES "/mnt/SDCARD/Stories/"

void stories_autosleep_unlock(void) {
    autosleep_unlock(parameters_getScreenOnInactivityTime(), parameters_getScreenOffInactivityTime());
}

void stories_saveSession(void)
{
    file_save(
        APP_SAVEFILE, 
        "{\"app\":%d, \"storyIndex\":%d, \"storyActionKey\":\"%s\", \"storyActionOptionIndex\":%d, \"storyTime\":%lf}", 
        APP_STORIES,
        storyIndex, 
        storyActionKey,
        storyActionOptionIndex,
        storyAutoplay ? storyTime : 0
    );
}

bool stories_loadSession(void)
{
    storyStageKey[0] = '\0';
    cJSON *savedState = json_load(APP_SAVEFILE);
    int a;
    if(savedState != NULL && json_getInt(savedState, "app", &a) && a == APP_STORIES) {
        json_getInt(savedState, "storyIndex", &storyIndex);
        json_getString(savedState, "storyActionKey", storyActionKey);
        json_getInt(savedState, "storyActionOptionIndex", &storyActionOptionIndex);
        json_getDouble(savedState, "storyTime", &storyTime);
        remove(APP_SAVEFILE);
        return true;
    }
    storyActionKey[0] = '\0';
    storyActionOptionIndex = 0;
    storyTime = 0;
    return false;
}


cJSON *stories_getStage(void)
{
    cJSON *stageNodes = cJSON_GetObjectItem(storyJson, "stages");

    if(stageNodes == NULL) {
        return NULL;
    }

    return cJSON_GetObjectItem(stageNodes, storyStageKey);
}

cJSON *stories_getAction(void)
{
    if(storyActionKey[0] == '\0') {
        return NULL;
    }

    cJSON *actionNodes = cJSON_GetObjectItem(storyJson, "actions");

    if(actionNodes == NULL) {
        return NULL;
    }

    return cJSON_GetObjectItem(actionNodes, storyActionKey);
}

void stories_readStage(void)
{
    cJSON *stageNode = stories_getStage();
    if(stageNode == NULL) {
        return;
    }
    
    char story_audio_path[STR_MAX], story_image_path[STR_MAX], imageFilename[STR_MAX], soundFilename[STR_MAX];
    sprintf(story_audio_path, "%s%s/audios/", STORIES_RESOURCES, storiesList[storyIndex]);
    sprintf(story_image_path, "%s%s/images/", STORIES_RESOURCES, storiesList[storyIndex]);
    
    if(!cJSON_IsNull(cJSON_GetObjectItem(stageNode, "image")) && json_getString(stageNode, "image", imageFilename)) {
        video_displayImage(story_image_path, imageFilename);
        display_setScreen(true);
    } else {
        video_displayBlackScreen();
        display_setScreen(false);
    }

    storyAutoplay = false;
    storyOkAction = true;

    if(!cJSON_IsNull(cJSON_GetObjectItem(stageNode, "audio")) && json_getString(stageNode, "audio", soundFilename)) {
        audio_play(story_audio_path, soundFilename, storyTime);
        storyStartTime = get_time();
        if(cJSON_IsTrue(cJSON_GetObjectItem(cJSON_GetObjectItem(stageNode, "control"), "autoplay"))) {
            storyAutoplay = true;
            storyOkAction = cJSON_IsTrue(cJSON_GetObjectItem(cJSON_GetObjectItem(stageNode, "control"), "ok"));
            autosleep_lock();
            Mix_HookMusicFinished(callback_stories_autoplay);
        } else {
            stories_autosleep_unlock();
        }
    } else {
        stories_autosleep_unlock();
    }
}

void stories_readAction(void)
{
    if(storiesCount == 0) {
        return;
    }
    
    if(storyActionOptionIndex < 0) {
        storyActionOptionIndex = storyActionOptionsCount - 1;
    } else if (storyActionOptionIndex >= storyActionOptionsCount) {
        storyActionOptionIndex = 0;
    }

    cJSON *nodeAction = stories_getAction();
    if(nodeAction == NULL) {
        return;
    }

    cJSON *option = cJSON_GetArrayItem(nodeAction, storyActionOptionIndex);
    
    if(option == NULL) {
        return;
    }

    json_getString(option, "stage", storyStageKey);
    stories_readStage();
}

void stories_load(void)
{
    if(storiesCount == 0) {
        return;
    }

    char story_path[STR_MAX];
    sprintf(story_path, "%s%s/nodes.json", STORIES_RESOURCES, storiesList[storyIndex]);
    storyJson = json_load(story_path);
    if(storyJson == NULL) {
        return;
    }

    if(storyActionKey[0] == '\0') {
        cJSON *startTransition = cJSON_GetObjectItem(storyJson, "startAction");

        if(startTransition == NULL) {
            return callback_stories_reset();
        }

        json_getString(startTransition, "action", storyActionKey);
        json_getInt(startTransition, "index", &storyActionOptionIndex);
        storyTime = 0;
    }

    cJSON *nodeAction = stories_getAction();
    if(nodeAction == NULL) {
        return;
    }

    storyActionOptionsCount = cJSON_GetArraySize(nodeAction);
    stories_readAction();
}

void stories_title(void)
{
    if(storiesCount == 0) {
        video_displayImage(SYSTEM_RESOURCES, "noStory.png");
        return;
    }

    if(stories_loadSession()) {
        return stories_load();
    }

    if(storyIndex < 0) {
        storyIndex = storiesCount - 1;
    } else if (storyIndex >= storiesCount) {
        storyIndex = 0;
    }
    
    char story_path[STR_MAX];
    sprintf(story_path, "%s%s/", STORIES_RESOURCES, storiesList[storyIndex]);
    
    storyTime = 0;
    video_displayImage(story_path, "title.png");
    audio_play(story_path, "title.mp3", storyTime);
    display_setScreen(true);
    stories_autosleep_unlock();
    storyStartTime = get_time();
}

void stories_transition(char* transition) {
    
    if(storiesCount == 0) {
        return;
    }

    cJSON *stageNode = stories_getStage();
    if(stageNode == NULL) {
        return callback_stories_reset();
    }
    
    cJSON *transitionNode = cJSON_GetObjectItem(stageNode, transition);
    if(transitionNode == NULL || cJSON_IsNull(transitionNode)) {
        return callback_stories_reset();
    }

    Mix_HookMusicFinished(NULL);
    storyTime = 0;

    json_getString(transitionNode, "action", storyActionKey);
    json_getInt(transitionNode, "index", &storyActionOptionIndex);
    storyActionOptionsCount = cJSON_GetArraySize(stories_getAction());
    stories_readAction();
}

void stories_rewind(double time)
{
    long int ts = get_time();
    storyTime += (double)(ts - storyStartTime) + time;
    storyStartTime = ts;
    audio_setPosition(storyTime);
}

void stories_next(void)
{
    if(storyAutoplay) {
        stories_rewind(10);
    } else {
        if(storyActionKey[0] == '\0') {
            storyIndex += 1;
            stories_title();
        } else {
            storyActionOptionIndex += 1;
            stories_readAction();
        }
    }
    
}

void stories_previous(void)
{
    if(storyAutoplay) {
        stories_rewind(-10);
    } else {
        if(storyActionKey[0] == '\0') {
            storyIndex -= 1;
            stories_title();
        } else {
            storyActionOptionIndex -= 1;
            stories_readAction();
        }
    }
}

void stories_ok(void)
{
    if(!storyOkAction) {
        return;
    }
    
    if(storyActionKey[0] == '\0') {
        stories_load();
    } else {
        stories_transition("ok");
    }
}


void stories_autoplay(void)
{
    storyOkAction = true;
    stories_ok();
}


void stories_pause(void)
{
    if(Mix_PlayingMusic() == 1) {
        if (Mix_PausedMusic() == 1) {
            autosleep_lock();
            Mix_ResumeMusic();
            storyStartTime = get_time();
        } else {
            stories_autosleep_unlock();
            Mix_PauseMusic();
            long int ts = get_time();
            storyTime += (double)(ts - storyStartTime);
            storyStartTime = ts;
        }
    }
}

bool stories_home(void)
{
    if(storyActionKey[0] == '\0') {
        Mix_HookMusicFinished(NULL);
        if(Mix_PlayingMusic() == 1) {
            if (Mix_PausedMusic() != 1) {
                Mix_PauseMusic();
            }
        }
        return true;
    } else {
        stories_transition("home");
        return false;
    }
}

void stories_save(void)
{
    if(Mix_PlayingMusic() == 1) {
        if (Mix_PausedMusic() != 1) {
            stories_pause();
        }
    }
    stories_saveSession();
}

void stories_reset(void) 
{
    Mix_HookMusicFinished(NULL);
    storyAutoplay = false;
    storyOkAction = true;
    storyStageKey[0] = '\0';
    storyActionKey[0] = '\0';
    storyActionOptionIndex = 0;
    storyTime = 0;
    if(storyJson != NULL) {
        cJSON_free(storyJson);
        storyJson = NULL;
    }
    stories_title();
}

void stories_init(void)
{
    if(storiesList != NULL) {
        return stories_title();
    }

    callback_stories_autoplay = &stories_autoplay;
    callback_stories_reset = &stories_reset;

    video_displayImage(SYSTEM_RESOURCES, "loadingStories.png");

    int i=0;
    DIR *d;
    struct dirent *dir;
    d = opendir(STORIES_RESOURCES);

    while((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            i++;
        }
    }
    rewinddir(d);

    storiesCount = i;
    char **filesList = (char**)malloc(i * sizeof(char*));
    i=0;

    while((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            filesList[i] = (char*) malloc(STR_DIRNAME);
            strcpy(filesList[i], dir->d_name);
            i++;
        }
    }
    closedir(d);

    if(storiesCount > 0) {
        sort(filesList, storiesCount);
    }

    storiesList = filesList;
    stories_title();
}

#endif // STORYTELLER_STORIES_HELPER__