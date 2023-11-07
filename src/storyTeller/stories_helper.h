#ifndef STORYTELLER_STORIES_HELPER__
#define STORYTELLER_STORIES_HELPER__

#include <string.h>
#include <dirent.h>
#include "system/display.h"
#include "utils/str.h"
#include "utils/json.h"
#include "./sdl_helper.h"

static char **storiesList = NULL;
static cJSON *jsonRoot = NULL;
static int storiesCount = 0;
static int storyIndex = 0;
static int stageIndex = 0;
static int actionIndex = -1;
static int actionOptionIndex = 0;
static int actionOptionsCount = 0;
static bool autoplay = false;
static void (*callback_stories_autoplay)(void);

#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"
#define STORIES_RESOURCES "/mnt/SDCARD/Stories/"

cJSON *stories_getStage(void)
{
    cJSON *stageNodes = cJSON_GetObjectItem(jsonRoot, "stageNodes");

    if(stageNodes == NULL) {
        return NULL;
    }

    return cJSON_GetArrayItem(stageNodes, stageIndex);
}

cJSON *stories_getAction(void)
{
    if(actionIndex == -1) {
        return NULL;
    }

    cJSON *actionNodes = cJSON_GetObjectItem(jsonRoot, "actionNodes");

    if(actionNodes == NULL) {
        return NULL;
    }

    return cJSON_GetArrayItem(actionNodes, actionIndex);
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
        audio_play(story_path, soundFilename);
        if(cJSON_IsTrue(cJSON_GetObjectItem(cJSON_GetObjectItem(stageNode, "controlSettings"), "autoplay"))) {
            autoplay = true;
            Mix_HookMusicFinished(callback_stories_autoplay);
        }
    }
}

void stories_readAction(void)
{
    if(storiesCount == 0) {
        return;
    }
    
    if(actionOptionIndex < 0) {
        actionOptionIndex = actionOptionsCount - 1;
    } else if (actionOptionIndex >= actionOptionsCount) {
        actionOptionIndex = 0;
    }

    cJSON *stageAction = stories_getAction();
    if(stageAction == NULL) {
        return;
    }
    
    cJSON *options = cJSON_GetObjectItem(stageAction, "options");
    if(options == NULL) {
        return;
    }
    
    cJSON *option = cJSON_GetArrayItem(options, actionOptionIndex);
    if(option == NULL) {
        return;
    }
    
    char *stageNodeUUID = cJSON_GetStringValue(option);
    char stageUUID[STR_MAX];
    stageIndex = -1;
    do {
        stageIndex += 1;
        cJSON *stageNode = stories_getStage();
        if(stageNode == NULL) {
            stageIndex = 0;
            actionIndex = -1;
            return;
        }
        json_getString(stageNode, "uuid", stageUUID);

    } while(strcmp(stageUUID, stageNodeUUID) != 0);
    
    stories_readStage();
}

void stories_reset(void) 
{
    Mix_HookMusicFinished(NULL);
    autoplay = false;
    stageIndex = 0;
    actionIndex = -1;
    stories_readStage();
}

void stories_load(void)
{
    if(storiesCount == 0) {
        video_displayImage(SYSTEM_RESOURCES, "noStory.png");
        return;
    }

    if(storyIndex < 0) {
        storyIndex = storiesCount - 1;
    } else if (storyIndex >= storiesCount) {
        storyIndex = 0;
    }
    
    char story_path[STR_MAX];
    sprintf(story_path, "%s%s/story.json", STORIES_RESOURCES, storiesList[storyIndex]);

    jsonRoot = json_load(story_path);
    if(jsonRoot == NULL) {
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
    json_getInt(transitionNode, "optionIndex", &actionOptionIndex);

    actionIndex = -1;
    do {
        actionIndex += 1;
        actionNode = stories_getAction();
        if(actionNode == NULL) {
            return stories_reset();
        }
        json_getString(actionNode, "id", actionId);

    } while(strcmp(actionId, actionNodeId) != 0);

    actionOptionsCount = cJSON_GetArraySize(cJSON_GetObjectItem(actionNode, "options"));
    return stories_readAction();
}

void stories_next(void)
{
    if(actionIndex == -1) {
        storyIndex += 1;
        stories_load();
    } else {
        actionOptionIndex += 1;
        stories_readAction();
    }
}

void stories_previous(void)
{
    if(actionIndex == -1) {
        storyIndex -= 1;
        stories_load();
    } else {
        actionOptionIndex -= 1;
        stories_readAction();
    }
}

void stories_ok(void)
{
    if(autoplay) {
        return;
    }
    
    stories_transition("okTransition");
}


void stories_autoplay(void)
{
    autoplay = false;
    stories_ok();
}


void stories_pause(void)
{
    if(Mix_PlayingMusic() == 1) {
        if (Mix_PausedMusic() == 1) {
            Mix_ResumeMusic();
        } else {
            Mix_PauseMusic();
        }
    }
}

void stories_home(void)
{
    stories_transition("homeTransition");
}

void stories_init(void)
{
    callback_stories_autoplay = &stories_autoplay;

    video_displayImage(SYSTEM_RESOURCES, "storiesLoading.png");

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
    stories_load();
}

#endif // STORYTELLER_STORIES_HELPER__