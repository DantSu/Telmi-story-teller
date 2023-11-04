#ifndef STORYTELLER_STORIES_HELPER__
#define STORYTELLER_STORIES_HELPER__

#include <dirent.h>
#include "utils/str.h"
#include "utils/file.h"
#include "utils/json.h"

static char **storiesList = NULL;
static cJSON *jsonRoot = NULL;
static int storiesCount = 0;
static int storyIndex;

#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"
#define STORIES_RESOURCES "/mnt/SDCARD/Stories/"

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

    jsonRoot = cJSON_Parse(json_str);
    if(jsonRoot == NULL) {
        return;
    }
    
    cJSON *stageNodes = cJSON_GetObjectItem(jsonRoot, "stageNodes");
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
    stories_show();
}

#endif // STORYTELLER_STORIES_HELPER__