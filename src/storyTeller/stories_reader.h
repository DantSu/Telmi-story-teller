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

static char **storiesList = NULL;
static cJSON *storyJson = NULL;
static int storiesCount = 0;
static int storyIndex = 0;
static int storyStageIndex = 0;
static int storyActionIndex = -1;
static int storyActionOptionIndex = 0;
static int storyActionOptionsCount = 0;
static double storyTime = 0;
static long int storyStartTime = 0;
static bool storyAutoplay = false;
static void (*callback_stories_autoplay)(void);

#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"
#define STORIES_RESOURCES "/mnt/SDCARD/Stories/"

long int stories_timestamp(void) {
    return (long int) time(0);
}

bool stories_saveSession(void)
{
    return file_save(
        APP_SAVEFILE, 
        "{\"app\":%d, \"storyIndex\":%d, \"storyStageIndex\":%d, \"storyActionIndex\":%d, \"storyActionOptionIndex\":%d, \"storyTime\":%lf}", 
        APP_STORIES,
        storyIndex, 
        storyStageIndex, 
        storyActionIndex,
        storyActionOptionIndex,
        storyAutoplay ? storyTime : 0
    );
}

bool stories_loadSession(void)
{
    cJSON *savedState = json_load(APP_SAVEFILE);
    int a;
    if(savedState != NULL && json_getInt(savedState, "app", &a) && a == APP_STORIES) {
        json_getInt(savedState, "storyIndex", &storyIndex);
        json_getInt(savedState, "storyStageIndex", &storyStageIndex);
        json_getInt(savedState, "storyActionIndex", &storyActionIndex);
        json_getInt(savedState, "storyActionOptionIndex", &storyActionOptionIndex);
        json_getDouble(savedState, "storyTime", &storyTime);
        remove(APP_SAVEFILE);
        return true;
    }

    storyStageIndex = 0;
    storyActionIndex = -1;
    storyActionOptionIndex = 0;
    storyTime = 0;
    return false;
}


cJSON *stories_getStage(void)
{
    cJSON *stageNodes = cJSON_GetObjectItem(storyJson, "stageNodes");

    if(stageNodes == NULL) {
        return NULL;
    }

    return cJSON_GetArrayItem(stageNodes, storyStageIndex);
}

cJSON *stories_getAction(void)
{
    if(storyActionIndex == -1) {
        return NULL;
    }

    cJSON *actionNodes = cJSON_GetObjectItem(storyJson, "actionNodes");

    if(actionNodes == NULL) {
        return NULL;
    }

    return cJSON_GetArrayItem(actionNodes, storyActionIndex);
}

void stories_readStage(void)
{
    cJSON *stageNode = stories_getStage();
    if(stageNode == NULL) {
        return;
    }
    
    char story_path[STR_MAX], imageFilename[STR_MAX], soundFilename[STR_MAX];
    sprintf(story_path, "%s%s/assets/", STORIES_RESOURCES, storiesList[storyIndex]);
    
    if(!cJSON_IsNull(cJSON_GetObjectItem(stageNode, "image")) && json_getString(stageNode, "image", imageFilename)) {
        video_displayImage(story_path, imageFilename);
        display_setScreen(true);
    } else {
        video_displayBlackScreen();
        display_setScreen(false);
    }

    if(!cJSON_IsNull(cJSON_GetObjectItem(stageNode, "audio")) && json_getString(stageNode, "audio", soundFilename)) {
        audio_play(story_path, soundFilename, &storyTime);
        storyStartTime = stories_timestamp();
        if(cJSON_IsTrue(cJSON_GetObjectItem(cJSON_GetObjectItem(stageNode, "controlSettings"), "autoplay"))) {
            storyAutoplay = true;
            autosleep_lock();
            Mix_HookMusicFinished(callback_stories_autoplay);
        } else {
            autosleep_unlock();
        }
    } else {
        autosleep_unlock();
    }
}

void stories_reset(void) 
{
    Mix_HookMusicFinished(NULL);
    storyAutoplay = false;
    storyStageIndex = 0;
    storyActionIndex = -1;
    storyActionOptionIndex = 0;
    storyTime = 0;
    stories_readStage();
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

    cJSON *stageAction = stories_getAction();
    if(stageAction == NULL) {
        return;
    }
    
    cJSON *options = cJSON_GetObjectItem(stageAction, "options");
    if(options == NULL) {
        return;
    }
    
    cJSON *option = cJSON_GetArrayItem(options, storyActionOptionIndex);
    if(option == NULL) {
        return;
    }
    
    char *stageNodeUUID = cJSON_GetStringValue(option);
    char stageUUID[STR_MAX];
    storyStageIndex = -1;
    do {
        storyStageIndex += 1;
        cJSON *stageNode = stories_getStage();
        if(stageNode == NULL) {
            return stories_reset();
        }
        json_getString(stageNode, "uuid", stageUUID);

    } while(strcmp(stageUUID, stageNodeUUID) != 0);
    
    storyTime = 0;
    stories_readStage();
}

void stories_load(void)
{
    if(storiesCount == 0) {
        video_displayImage(SYSTEM_RESOURCES, "noStory.png");
        return;
    }
    
    stories_loadSession();

    if(storyIndex < 0) {
        storyIndex = storiesCount - 1;
    } else if (storyIndex >= storiesCount) {
        storyIndex = 0;
    }

    char story_path[STR_MAX];
    sprintf(story_path, "%s%s/story.json", STORIES_RESOURCES, storiesList[storyIndex]);
    storyJson = json_load(story_path);
    if(storyJson == NULL) {
        return;
    }
    
    stories_readStage();
}

void stories_transition(char* transition) {
    if(storiesCount == 0) {
        return;
    }

    Mix_HookMusicFinished(NULL);

    cJSON *stageNode = stories_getStage();
    if(stageNode == NULL) {
        return stories_reset();
    }
    
    cJSON *transitionNode = cJSON_GetObjectItem(stageNode, transition);
    if(transitionNode == NULL || cJSON_IsNull(transitionNode)) {
        return stories_reset();
    }

    cJSON *actionNode;
    char actionNodeId[STR_MAX], actionId[STR_MAX];

    json_getString(transitionNode, "actionNode", actionNodeId);
    json_getInt(transitionNode, "optionIndex", &storyActionOptionIndex);

    storyActionIndex = -1;
    do {
        storyActionIndex += 1;
        actionNode = stories_getAction();
        if(actionNode == NULL) {
            return stories_reset();
        }
        json_getString(actionNode, "id", actionId);

    } while(strcmp(actionId, actionNodeId) != 0);

    storyActionOptionsCount = cJSON_GetArraySize(cJSON_GetObjectItem(actionNode, "options"));
    return stories_readAction();
}

void stories_rewind(double time)
{
    long int ts = stories_timestamp();
    storyTime += (double)(ts - storyStartTime) + time;
    storyStartTime = ts;
    Mix_SetMusicPosition(storyTime);
}

void stories_next(void)
{
    if(storyAutoplay) {
        stories_rewind(10);
    } else {
        if(storyActionIndex == -1) {
            storyIndex += 1;
            stories_load();
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
        if(storyActionIndex == -1) {
            storyIndex -= 1;
            stories_load();
        } else {
            storyActionOptionIndex -= 1;
            stories_readAction();
        }
    }
}

void stories_ok(void)
{
    if(storyAutoplay) {
        return;
    }
    
    stories_transition("okTransition");
}


void stories_autoplay(void)
{
    storyAutoplay = false;
    stories_ok();
}


void stories_pause(void)
{
    if(Mix_PlayingMusic() == 1) {
        if (Mix_PausedMusic() == 1) {
            autosleep_lock();
            Mix_ResumeMusic();
            storyStartTime = stories_timestamp();
        } else {
            autosleep_unlock();
            Mix_PauseMusic();
            long int ts = stories_timestamp();
            storyTime += (double)(ts - storyStartTime);
            storyStartTime = ts;
        }
    }
}

bool stories_home(void)
{
    if(storyActionIndex == -1) {
        Mix_HookMusicFinished(NULL);
        if(Mix_PlayingMusic() == 1) {
            if (Mix_PausedMusic() != 1) {
                Mix_PauseMusic();
            }
        }
        return true;
    } else {
        stories_transition("homeTransition");
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

void stories_init(void)
{
    callback_stories_autoplay = &stories_autoplay;

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
            filesList[i] = (char*) malloc(STR_MAX * sizeof(char));
            strcpy(filesList[i], dir->d_name);
            i++;
        }
    }
    closedir(d);
    storiesList = filesList;
    stories_load();
}

#endif // STORYTELLER_STORIES_HELPER__