#ifndef STORYTELLER_STORIES_HELPER__
#define STORYTELLER_STORIES_HELPER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "system/display.h"
#include "utils/str.h"
#include "utils/json.h"

#include "./app_file.h"
#include "./app_autosleep.h"
#include "./app_lock.h"
#include "./app_parameters.h"
#include "./sdl_helper.h"
#include "./array_helper.h"
#include "./time_helper.h"

#define SYSTEM_RESOURCES "/mnt/SDCARD/.tmp_update/res/"
#define STORIES_RESOURCES "/mnt/SDCARD/Stories/"

#define STORIES_DISPLAY_MODE_SINGLE 0
#define STORIES_DISPLAY_MODE_TILES 1

#define STR_DIRNAME 128

#define MAX_STORIES_NIGHT_MODE 16

static int storiesDiplayMode = STORIES_DISPLAY_MODE_SINGLE;
static bool storiesNightModeEnabled = false;
static bool storiesNightModePlaying = false;
static int storiesNightModeIndex = 0;
static char storiesNightModePlaylist[MAX_STORIES_NIGHT_MODE][STR_MAX * 2] = {{'\0'},
                                                                             {'\0'},
                                                                             {'\0'},
                                                                             {'\0'},
                                                                             {'\0'},
                                                                             {'\0'},
                                                                             {'\0'},
                                                                             {'\0'},
                                                                             {'\0'},
                                                                             {'\0'},
                                                                             {'\0'},
                                                                             {'\0'},
                                                                             {'\0'},
                                                                             {'\0'},
                                                                             {'\0'},
                                                                             {'\0'}};
static char storiesNightModeCurrentAudio[STR_MAX * 2] = {'\0'};

static char **storiesList = NULL;
static int storiesCount = 0;
static int storyIndex = 0;

static cJSON *storyJson = NULL;
static bool hasInventory = false;
static int storyInventoryCountLength = -1;
static int *storyInventoryCount = NULL;
static char storyStageKey[STR_MAX] = {'\0'};
static char storyActionKey[STR_MAX] = {'\0'};
static int storyActionOptionIndex = 0;
static int storyActionOptionsCount = 0;
static double storyTime = 0;
static long int storyStartTime = 0;
static bool storyScreenEnabled = true;
static bool storyAutoplay = false;
static bool storyOkAction = true;

static long storyLastPlayingTime = 0;
static int storyPlayingTime = 0;

static void (*callback_stories_autoplay)(void);

static void (*callback_stories_nightMode)(void);

static void (*callback_stories_reset)(void);

void stories_autosleep_unlock(void) {
    autosleep_unlock(parameters_getScreenOnInactivityTime(), parameters_getScreenOffInactivityTime());
}

void stories_initPlayingTime(void) {
    storyPlayingTime = 0;
    storyLastPlayingTime = get_time();
}

int stories_getPlayingTime(void) {
    long time = get_time();
    storyPlayingTime += time - storyLastPlayingTime;
    storyLastPlayingTime = time;
    return storyPlayingTime;
}

void stories_saveSession(void) {
    if (storiesNightModeEnabled) {
        if (storiesNightModePlaying) {
            file_save(
                    APP_SAVEFILE,
                    "{\"app\":%d, \"nightMode\":true, \"playlist\":[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"], \"storyIndex\":%d, \"storyTime\":%lf}",
                    APP_STORIES,
                    storiesNightModePlaylist[0],
                    storiesNightModePlaylist[1],
                    storiesNightModePlaylist[2],
                    storiesNightModePlaylist[3],
                    storiesNightModePlaylist[4],
                    storiesNightModePlaylist[5],
                    storiesNightModePlaylist[6],
                    storiesNightModePlaylist[7],
                    storiesNightModePlaylist[8],
                    storiesNightModePlaylist[9],
                    storiesNightModePlaylist[10],
                    storiesNightModePlaylist[11],
                    storiesNightModePlaylist[12],
                    storiesNightModePlaylist[13],
                    storiesNightModePlaylist[14],
                    storiesNightModePlaylist[15],
                    storiesNightModeIndex,
                    storyTime
            );
        }
    } else {
        char jsonInventory[STR_MAX * 2] = "";
        if (hasInventory) {
            for (int i = 0; i < storyInventoryCountLength; ++i) {
                if (i == 0) {
                    sprintf(jsonInventory, "%d", storyInventoryCount[i]);
                } else {
                    char tmpJsonInventory[8];
                    sprintf(tmpJsonInventory, ",%d", storyInventoryCount[i]);
                    strcat(jsonInventory, tmpJsonInventory);
                }
            }
        }
        file_save(
                APP_SAVEFILE,
                "{\"app\":%d, \"storyIndex\":%d, \"storyActionKey\":\"%s\", \"storyActionOptionIndex\":%d, \"storyTime\":%lf, \"storyPlayingTime\":%d, \"inventory\":[%s]}",
                APP_STORIES,
                storyIndex,
                storyActionKey,
                storyActionOptionIndex,
                storyAutoplay ? storyTime : 0,
                stories_getPlayingTime(),
                jsonInventory
        );
    }
}

bool stories_loadSession(void) {
    storyStageKey[0] = '\0';
    stories_initPlayingTime();
    cJSON *savedState = json_load(APP_SAVEFILE);
    int a;
    bool b;
    if (savedState != NULL && json_getInt(savedState, "app", &a) && a == APP_STORIES) {
        if (json_getBool(savedState, "nightMode", &b) && b) {
            storiesNightModeEnabled = true;
            cJSON *playlistArray = cJSON_GetObjectItem(savedState, "playlist");
            strcpy(storiesNightModePlaylist[0], cJSON_GetStringValue(cJSON_GetArrayItem(playlistArray, 0)));
            strcpy(storiesNightModePlaylist[1], cJSON_GetStringValue(cJSON_GetArrayItem(playlistArray, 1)));
            strcpy(storiesNightModePlaylist[2], cJSON_GetStringValue(cJSON_GetArrayItem(playlistArray, 2)));
            strcpy(storiesNightModePlaylist[3], cJSON_GetStringValue(cJSON_GetArrayItem(playlistArray, 3)));
            strcpy(storiesNightModePlaylist[4], cJSON_GetStringValue(cJSON_GetArrayItem(playlistArray, 4)));
            strcpy(storiesNightModePlaylist[5], cJSON_GetStringValue(cJSON_GetArrayItem(playlistArray, 5)));
            strcpy(storiesNightModePlaylist[6], cJSON_GetStringValue(cJSON_GetArrayItem(playlistArray, 6)));
            strcpy(storiesNightModePlaylist[7], cJSON_GetStringValue(cJSON_GetArrayItem(playlistArray, 7)));
            strcpy(storiesNightModePlaylist[8], cJSON_GetStringValue(cJSON_GetArrayItem(playlistArray, 8)));
            strcpy(storiesNightModePlaylist[9], cJSON_GetStringValue(cJSON_GetArrayItem(playlistArray, 9)));
            strcpy(storiesNightModePlaylist[10], cJSON_GetStringValue(cJSON_GetArrayItem(playlistArray, 10)));
            strcpy(storiesNightModePlaylist[11], cJSON_GetStringValue(cJSON_GetArrayItem(playlistArray, 11)));
            strcpy(storiesNightModePlaylist[12], cJSON_GetStringValue(cJSON_GetArrayItem(playlistArray, 12)));
            strcpy(storiesNightModePlaylist[13], cJSON_GetStringValue(cJSON_GetArrayItem(playlistArray, 13)));
            strcpy(storiesNightModePlaylist[14], cJSON_GetStringValue(cJSON_GetArrayItem(playlistArray, 14)));
            strcpy(storiesNightModePlaylist[15], cJSON_GetStringValue(cJSON_GetArrayItem(playlistArray, 15)));
            json_getInt(savedState, "storyIndex", &storiesNightModeIndex);
        } else {
            json_getInt(savedState, "storyIndex", &storyIndex);
            json_getString(savedState, "storyActionKey", storyActionKey);
            json_getInt(savedState, "storyActionOptionIndex", &storyActionOptionIndex);
            json_getInt(savedState, "storyPlayingTime", &storyPlayingTime);

            cJSON *inventoryState = cJSON_GetObjectItem(savedState, "inventory");
            if (inventoryState != NULL && cJSON_IsArray(inventoryState) && cJSON_GetArraySize(inventoryState) > 0) {
                storyInventoryCountLength = cJSON_GetArraySize(inventoryState);
                storyInventoryCount = malloc(storyInventoryCountLength * sizeof(int));
                for (int i = 0; i < storyInventoryCountLength; ++i) {
                    storyInventoryCount[i] = cJSON_GetNumberValue(cJSON_GetArrayItem(inventoryState, i));
                }
            } else {
                storyInventoryCount = NULL;
                storyInventoryCountLength = -1;
            }
        }
        json_getDouble(savedState, "storyTime", &storyTime);
        remove(APP_SAVEFILE);
        return true;
    }
    storyActionKey[0] = '\0';
    storyActionOptionIndex = 0;
    storyTime = 0;
    return false;
}

void stories_nightMode_reset(void) {
    storiesNightModeEnabled = false;
    storiesNightModePlaying = false;
    storiesNightModeIndex = 0;
    for (int i = 0; i < MAX_STORIES_NIGHT_MODE; ++i) {
        storiesNightModePlaylist[i][0] = '\0';
    }
}

int stories_nightMode_count(void) {
    for (int i = 0; i < MAX_STORIES_NIGHT_MODE; ++i) {
        if (storiesNightModePlaylist[i][0] == '\0') {
            return i;
        }
    }
    return MAX_STORIES_NIGHT_MODE;
}

void stories_nightMode_screenDisplayCount(void) {
    char writePlaylist[STR_MAX];
    sprintf(writePlaylist, "%i", stories_nightMode_count());
    video_screenAddImage(SYSTEM_RESOURCES, "storytellerNightModeCounter.png", 33, 2, 64);
    video_screenWriteFont(writePlaylist, fontBold20, colorPurple, 74, 1, SDL_ALIGN_CENTER);
}

bool stories_nightMode_isFull(void) {
    return storiesNightModePlaylist[MAX_STORIES_NIGHT_MODE - 1][0] != '\0';
}

bool stories_nightMode_isPlaylistableAudio(void) {
    return storiesNightModeCurrentAudio[0] != '\0';
}

bool stories_nightMode_addToPlaylist(void) {
    if (stories_nightMode_isFull()) {
        return true;
    }
    strcpy(storiesNightModePlaylist[stories_nightMode_count()], storiesNightModeCurrentAudio);
    return stories_nightMode_isFull();
}

void stories_nightMode_play(void) {
    audio_play_path(storiesNightModePlaylist[storiesNightModeIndex], storyTime);
    storyStartTime = get_time();
    Mix_HookMusicFinished(callback_stories_nightMode);
}

void stories_nightMode_resume(void) {
    autosleep_lock();

    video_displayBlackScreen();
    display_setScreen(false);
    storyScreenEnabled = false;

    storiesNightModePlaying = true;
    storiesNightModeCurrentAudio[0] = '\0';
    stories_nightMode_play();
}

void stories_nightMode_start(void) {
    storyAutoplay = false;
    storiesNightModeIndex = 0;
    storyTime = 0;
    stories_nightMode_resume();
}

void stories_nightMode_next(void) {
    ++storiesNightModeIndex;
    if (stories_nightMode_count() > storiesNightModeIndex) {
        storyTime = 0;
        stories_nightMode_play();
    } else {
        storiesNightModePlaying = false;
        autosleep_unlock(5, 5);
    }
}

void stories_inventory_screenDraw(void) {
    int itemsDisplayCount = 0;

    cJSON *inventory = cJSON_GetObjectItem(storyJson, "inventory");

    for (int i = 0; i < storyInventoryCountLength; ++i) {
        if (storyInventoryCount[i] > 0 &&
            ((int) cJSON_GetNumberValue(cJSON_GetObjectItem(cJSON_GetArrayItem(inventory, i), "display"))) < 2) {
            ++itemsDisplayCount;
        }
    }

    if (itemsDisplayCount == 0) {
        return;
    }

    video_screenAddImage(SYSTEM_RESOURCES, "storytellerInventoryBg.png", 0, 384, 640);

    if (itemsDisplayCount > 8) {
        itemsDisplayCount = 8;
    }

    int itemWidth = 80;
    int width = itemsDisplayCount * itemWidth;
    int x = 320 + width / 2;
    int y = 400;
    char storyPath[STR_MAX];
    sprintf(storyPath, "%s%s/images/", STORIES_RESOURCES, storiesList[storyIndex]);

    for (int i = 0; i < storyInventoryCountLength; ++i) {
        if (storyInventoryCount[i] <= 0) {
            continue;
        }

        cJSON *item = cJSON_GetArrayItem(inventory, i);
        int display = (int) cJSON_GetNumberValue(cJSON_GetObjectItem(item, "display"));
        if (display == 2) {
            continue;
        }

        int xItem = x - itemsDisplayCount * itemWidth;
        video_screenAddImage(storyPath, cJSON_GetStringValue(cJSON_GetObjectItem(item, "image")), xItem, y, itemWidth);

        switch (display) {
            case 0: {
                if (storyInventoryCount[i] > 1) {
                    char strCountItem[STR_MAX];
                    sprintf(strCountItem, "%d", storyInventoryCount[i]);
                    video_screenWriteFont(strCountItem,
                                          fontBold24,
                                          colorWhite,
                                          xItem + itemWidth - 2,
                                          y + itemWidth - 27,
                                          SDL_ALIGN_RIGHT);
                }
                break;
            }
            case 1: {
                int maxNumber = (int) cJSON_GetNumberValue(cJSON_GetObjectItem(item, "maxNumber"));
                video_drawRectangle(xItem + 4, y + 73, 72, 3, 75, 75, 75);
                video_drawRectangle(xItem + 4,
                                    y + 73,
                                    storyInventoryCount[i] * 72 / maxNumber,
                                    3,
                                    230,
                                    230,
                                    230);
                video_screenAddImage(
                        SYSTEM_RESOURCES,
                        "storytellerInventoryBar.png",
                        xItem + 2,
                        y + 71,
                        76);
                break;
            }
        }

        --itemsDisplayCount;
        if (itemsDisplayCount == 0) {
            break;
        }
    }
}

int stories_inventory_updateGetValue(int type, int number, int itemNumber, int maxNumber) {
    switch (type) {
        case 0: {
            itemNumber += number;
            break;
        }
        case 1: {
            itemNumber -= number;
            break;
        }
        case 2: {
            itemNumber = number;
            break;
        }
        case 3: {
            itemNumber *= number;
            break;
        }
        case 4: {
            itemNumber /= number;
            break;
        }
        case 5: {
            itemNumber %= number;
            break;
        }
    }
    if (itemNumber > maxNumber) {
        return maxNumber;
    }
    if (itemNumber < 0) {
        return 0;
    }
    return itemNumber;
}

int stories_inventory_update_getNumber(cJSON *updateItem) {
    int number = 0;
    if (json_getInt(updateItem, "number", &number)) {
        return number;
    }
    int item = 0;
    if (json_getInt(updateItem, "assignItem", &item)) {
        return storyInventoryCount[item];
    }
    bool playingTime = false;
    if (json_getBool(updateItem, "playingTime", &playingTime) && playingTime) {
        return stories_getPlayingTime();
    }
    return 0;
}

void stories_inventory_update(cJSON *node) {
    cJSON *inventory = cJSON_GetObjectItem(storyJson, "inventory");
    cJSON *updateItems = cJSON_GetObjectItem(node, "items");
    if (updateItems != NULL && cJSON_IsArray(updateItems) && cJSON_GetArraySize(updateItems) > 0) {
        int updateItemsSize = cJSON_GetArraySize(updateItems);
        for (int i = 0; i < updateItemsSize; ++i) {
            cJSON *updateItem = cJSON_GetArrayItem(updateItems, i);
            int index = 0;
            int type = 0;
            json_getInt(updateItem, "item", &index);
            json_getInt(updateItem, "type", &type);
            storyInventoryCount[index] = stories_inventory_updateGetValue(
                    type,
                    stories_inventory_update_getNumber(updateItem),
                    storyInventoryCount[index],
                    (int) cJSON_GetNumberValue(cJSON_GetObjectItem(cJSON_GetArrayItem(inventory, index), "maxNumber"))
            );
        }
    }
}

bool stories_inventory_test(int comparator, int conditionNumber, int itemNumber) {
    switch (comparator) {
        case 0: {
            return itemNumber < conditionNumber;
        }
        case 1: {
            return itemNumber <= conditionNumber;
        }
        case 2: {
            return itemNumber == conditionNumber;
        }
        case 3: {
            return itemNumber > conditionNumber;
        }
        case 4: {
            return itemNumber >= conditionNumber;
        }
        case 5: {
            return itemNumber != conditionNumber;
        }
    }
    return false;
}


int stories_inventory_testNode_getNumber(cJSON *condition) {
    int number = 0;
    if (json_getInt(condition, "number", &number)) {
        return number;
    }
    int item = 0;
    if (json_getInt(condition, "compareItem", &item)) {
        return storyInventoryCount[item];
    }
    return 0;
}

bool stories_inventory_testNode(cJSON *node) {
    cJSON *conditions = cJSON_GetObjectItem(node, "conditions");

    if (conditions == NULL || !cJSON_IsArray(conditions) || cJSON_GetArraySize(conditions) == 0) {
        return true;
    }

    int conditionsSize = cJSON_GetArraySize(conditions);
    for (int i = 0; i < conditionsSize; ++i) {
        cJSON *condition = cJSON_GetArrayItem(conditions, i);
        int index = 0;
        int comparator = 0;
        json_getInt(condition, "item", &index);
        json_getInt(condition, "comparator", &comparator);
        if (!stories_inventory_test(
                comparator,
                stories_inventory_testNode_getNumber(condition),
                storyInventoryCount[index]
        )) {
            return false;
        }
    }
    return true;
}

cJSON *stories_getStage(void) {
    cJSON *stageNodes = cJSON_GetObjectItem(storyJson, "stages");

    if (stageNodes == NULL) {
        return NULL;
    }

    return cJSON_GetObjectItem(stageNodes, storyStageKey);
}

cJSON *stories_getAction(void) {
    if (storyActionKey[0] == '\0') {
        return NULL;
    }

    cJSON *actionNodes = cJSON_GetObjectItem(storyJson, "actions");

    if (actionNodes == NULL) {
        return NULL;
    }

    return cJSON_GetObjectItem(actionNodes, storyActionKey);
}

void stories_readStage(void) {
    cJSON *stageNode = stories_getStage();
    if (stageNode == NULL) {
        return;
    }

    if (hasInventory) {
        stories_inventory_update(stageNode);
    }

    cJSON *controlJson = cJSON_GetObjectItem(stageNode, "control");
    storyAutoplay = cJSON_IsTrue(cJSON_GetObjectItem(controlJson, "autoplay"));
    storyOkAction = true;

    cJSON *audioJson = cJSON_GetObjectItem(stageNode, "audio");
    bool isAudioDefined = audioJson != NULL && cJSON_IsString(audioJson);

    if (!isAudioDefined && storyAutoplay) {
        audio_free_music();
        storyOkAction = false;
        storyScreenEnabled = false;

        if (storiesNightModeEnabled) {
            storiesNightModeCurrentAudio[0] = '\0';
        }

        callback_stories_autoplay();
        return;
    }

    cJSON *imageJson = cJSON_GetObjectItem(stageNode, "image");
    bool isImageDefined = imageJson != NULL && cJSON_IsString(imageJson);

    char story_audio_path[STR_MAX], story_image_path[STR_MAX];
    sprintf(story_audio_path, "%s%s/audios/", STORIES_RESOURCES, storiesList[storyIndex]);
    sprintf(story_image_path, "%s%s/images/", STORIES_RESOURCES, storiesList[storyIndex]);

    if (isAudioDefined) {
        audio_play(story_audio_path, cJSON_GetStringValue(audioJson), storyTime);
        storyStartTime = get_time();
        if (storyAutoplay) {
            storyOkAction = cJSON_IsTrue(cJSON_GetObjectItem(controlJson, "ok"));
            autosleep_lock();
            Mix_HookMusicFinished(callback_stories_autoplay);
        } else {
            stories_autosleep_unlock();
        }
    } else {
        audio_free_music();
        stories_autosleep_unlock();
    }

    if (isImageDefined) {
        display_setScreen(true);
        storyScreenEnabled = true;
        video_screenBlack();
        video_screenAddImage(story_image_path, cJSON_GetStringValue(imageJson), 0, 0, 640);
        if (hasInventory) {
            stories_inventory_screenDraw();
        }
        if (storiesNightModeEnabled) {
            storiesNightModeCurrentAudio[0] = '\0';
            stories_nightMode_screenDisplayCount();
        }
        video_applyToVideo();
    } else {
        storyScreenEnabled = false;
        if (storiesNightModeEnabled && storyAutoplay && !storyOkAction) {
            sprintf(storiesNightModeCurrentAudio, "%s%s", story_audio_path, cJSON_GetStringValue(audioJson));
            display_setScreen(true);
            video_screenBlack();
            video_screenAddImage(SYSTEM_RESOURCES, "storytellerNightMode.png", 0, 0, 640);
            stories_nightMode_screenDisplayCount();
            video_applyToVideo();
        } else {
            video_displayBlackScreen();
            if (!applock_isLockRecentlyChanged() && !applock_isUnlocking()) {
                display_setScreen(false);
            }
            if (storiesNightModeEnabled) {
                storiesNightModeCurrentAudio[0] = '\0';
            }
        }
    }
}

void stories_readAction(int direction) {
    if (storiesCount == 0) {
        return;
    }

    storyActionOptionIndex += direction;

    if (direction == 0) {
        direction = 1;
    }

    cJSON *nodeAction = stories_getAction();
    if (nodeAction == NULL) {
        return;
    }

    int initialOptionIndex = -1;

    while (true) {
        if (storyActionOptionIndex < 0) {
            storyActionOptionIndex = storyActionOptionsCount - 1;
        } else if (storyActionOptionIndex >= storyActionOptionsCount) {
            storyActionOptionIndex = 0;
        }

        if (initialOptionIndex == storyActionOptionIndex) {
            return callback_stories_reset();
        }

        if (initialOptionIndex == -1) {
            initialOptionIndex = storyActionOptionIndex;
        }

        cJSON *option = cJSON_GetArrayItem(nodeAction, storyActionOptionIndex);

        if (option == NULL || (hasInventory && !stories_inventory_testNode(option))) {
            storyActionOptionIndex += direction;
            continue;
        }

        json_getString(option, "stage", storyStageKey);
        stories_readStage();
        break;
    }
}

void stories_loadAction(void) {
    cJSON *nodeAction = stories_getAction();

    if (nodeAction == NULL || !cJSON_IsArray(nodeAction)) {
        return callback_stories_reset();
    }

    storyActionOptionsCount = cJSON_GetArraySize(nodeAction);

    if (storyActionOptionsCount == 0) {
        return callback_stories_reset();
    }

    if (storyActionOptionIndex < 0) {
        if (!hasInventory) {
            storyActionOptionIndex = rand() % storyActionOptionsCount;
        } else {
            int countOptions = 0;
            int indexesOptions[storyActionOptionsCount];
            for (int i = 0; i < storyActionOptionsCount; ++i) {
                cJSON *option = cJSON_GetArrayItem(nodeAction, i);
                if (stories_inventory_testNode(option)) {
                    indexesOptions[countOptions] = i;
                    ++countOptions;
                }
            }
            if (countOptions == 0) {
                return callback_stories_reset();
            }
            storyActionOptionIndex = indexesOptions[rand() % countOptions];
        }
    }

    if (storyActionOptionIndex >= storyActionOptionsCount) {
        storyActionOptionIndex = storyActionOptionsCount - 1;
    }

    stories_readAction(0);
}

void stories_loadAction_keyIndex(cJSON *transition) {
    storyTime = 0;
    json_getString(transition, "action", storyActionKey);
    if (json_getInt(transition, "index", &storyActionOptionIndex)) {
        return;
    }
    int item = 0;
    if (json_getInt(transition, "indexItem", &item)) {
        storyActionOptionIndex = storyInventoryCount[item];
        return;
    }
    storyActionOptionIndex = 0;
}

void stories_load(void) {
    if (storiesCount == 0 || storiesCount <= storyIndex) {
        return;
    }

    char story_path[STR_MAX];
    sprintf(story_path, "%s%s/nodes.json", STORIES_RESOURCES, storiesList[storyIndex]);
    storyJson = json_load(story_path);
    if (storyJson == NULL) {
        return;
    }

    cJSON *inventory = cJSON_GetObjectItem(storyJson, "inventory");
    hasInventory = inventory != NULL && cJSON_IsArray(inventory) && cJSON_GetArraySize(inventory) > 0;
    if (hasInventory) {
        int inventorySize = cJSON_GetArraySize(inventory);
        if (storyInventoryCount != NULL && storyInventoryCountLength != inventorySize) {
            free(storyInventoryCount);
            storyInventoryCount = NULL;
            storyInventoryCountLength = -1;
        }
        if (storyInventoryCount == NULL) {
            storyInventoryCountLength = inventorySize;
            storyInventoryCount = malloc(storyInventoryCountLength * sizeof(int));
            for (int i = 0; i < storyInventoryCountLength; ++i) {
                storyInventoryCount[i] = cJSON_GetNumberValue(
                        cJSON_GetObjectItem(cJSON_GetArrayItem(inventory, i), "initialNumber")
                );
            }
        }
    }

    if (storyActionKey[0] == '\0') {
        stories_initPlayingTime();

        cJSON *startTransition = cJSON_GetObjectItem(storyJson, "startAction");

        if (startTransition == NULL) {
            return callback_stories_reset();
        }

        stories_loadAction_keyIndex(startTransition);
    }

    stories_loadAction();
}

void stories_title_single(void) {
    char story_path[STR_MAX];
    sprintf(story_path, "%s%s/", STORIES_RESOURCES, storiesList[storyIndex]);
    video_displayImage(story_path, "title.png");
}

void stories_title_tile(int base, int pos) {
    int sIndex = base + pos;

    if (sIndex >= storiesCount) {
        return;
    }

    int x = 33 + (pos % 3) * 201;
    int y = 28 + (pos / 3) * 148;

    if (storyIndex == sIndex) {
        video_drawRectangle(x - 5, y - 5, 181, 138, 255, 186, 0);
    }
    char story_path[STR_MAX];
    sprintf(story_path, "%s%s/", STORIES_RESOURCES, storiesList[sIndex]);
    video_drawRectangle(x, y, 171, 128, 0, 0, 0);
    video_screenAddImage(story_path, "title.png", x, y, 171);
}

void stories_title_tiles(void) {
    int page = storyIndex / 9;
    int baseIndex = page * 9;
    char writePage[STR_MAX];
    sprintf(writePage, "%i / %i", storyIndex + 1, storiesCount);

    video_screenAddImage(SYSTEM_RESOURCES, "storiesTiles.png", 0, 0, 640);
    stories_title_tile(baseIndex, 0);
    stories_title_tile(baseIndex, 1);
    stories_title_tile(baseIndex, 2);
    stories_title_tile(baseIndex, 3);
    stories_title_tile(baseIndex, 4);
    stories_title_tile(baseIndex, 5);
    stories_title_tile(baseIndex, 6);
    stories_title_tile(baseIndex, 7);
    stories_title_tile(baseIndex, 8);
    if (storiesNightModeEnabled) {
        stories_nightMode_screenDisplayCount();
    }
    video_screenWriteFont(writePage, fontRegular16, colorWhite60, 606, 456, SDL_ALIGN_RIGHT);
    video_applyToVideo();
}

void stories_title(void) {
    if (storiesCount == 0) {
        video_displayImage(SYSTEM_RESOURCES, "noStory.png");
        return;
    }

    if (stories_loadSession()) {
        if (storiesNightModeEnabled) {
            return stories_nightMode_resume();
        } else if (storyActionKey[0] != '\0') {
            return stories_load();
        }
    }

    if (storyIndex < 0) {
        storyIndex = storiesCount - 1;
    } else if (storyIndex >= storiesCount) {
        storyIndex = 0;
    }

    stories_autosleep_unlock();
    storyStartTime = get_time();
    storyAutoplay = false;
    storyTime = 0;

    char story_path[STR_MAX];
    sprintf(story_path, "%s%s/", STORIES_RESOURCES, storiesList[storyIndex]);
    audio_play(story_path, "title.mp3", storyTime);

    storyScreenEnabled = true;
    display_setScreen(storyScreenEnabled);
    if (storiesDiplayMode == STORIES_DISPLAY_MODE_SINGLE) {
        stories_title_single();
    } else {
        stories_title_tiles();
    }
}

void stories_transition(char *transition) {
    if (storiesCount == 0) {
        return;
    }

    cJSON *stageNode = stories_getStage();
    if (stageNode == NULL) {
        return callback_stories_reset();
    }

    cJSON *transitionNode = cJSON_GetObjectItem(stageNode, transition);
    if (transitionNode == NULL || cJSON_IsNull(transitionNode)) {
        return callback_stories_reset();
    }

    Mix_HookMusicFinished(NULL);
    stories_loadAction_keyIndex(transitionNode);
    stories_loadAction();
}


void stories_setMode(int dm) {
    storiesDiplayMode = dm;
    if (storiesDiplayMode == STORIES_DISPLAY_MODE_SINGLE) {
        stories_title_single();
    } else {
        stories_title_tiles();
    }
}

void stories_changeTitle(int direction) {
    if (!storyAutoplay && storyActionKey[0] == '\0') {
        storyIndex += direction;
        stories_title();
    }
}

void stories_up(void) {
    if (storiesCount == 0 || storiesNightModePlaying) {
        return;
    }

    if (storiesDiplayMode == STORIES_DISPLAY_MODE_TILES) {
        stories_changeTitle(-3);
    }
}

void stories_down(void) {
    if (storiesCount == 0 || storiesNightModePlaying) {
        return;
    }

    if (storiesDiplayMode == STORIES_DISPLAY_MODE_TILES) {
        stories_changeTitle(3);
    }
}

void stories_rewind(double time) {
    long int ts = get_time();
    storyTime += (double) (ts - storyStartTime) + time;
    storyStartTime = ts;
    audio_setPosition(storyTime);
}

void stories_next(void) {
    if (storiesCount == 0) {
        return;
    }

    if (storyAutoplay || storiesNightModePlaying) {
        stories_rewind(10);
    } else {
        if (storyActionKey[0] == '\0') {
            stories_changeTitle(1);
        } else {
            stories_readAction(1);
        }
    }
}

void stories_previous(void) {
    if (storiesCount == 0) {
        return;
    }

    if (storyAutoplay || storiesNightModePlaying) {
        stories_rewind(-10);
    } else {
        if (storyActionKey[0] == '\0') {
            stories_changeTitle(-1);
        } else {
            stories_readAction(-1);
        }
    }
}

void stories_lockChanged(void) {
    if (applock_isLockRecentlyChanged() || applock_isUnlocking()) {
        display_setScreen(true);
    } else {
        display_setScreen(storyScreenEnabled);
    }
    video_applyToVideo();
}

void stories_menu(void) {
    if (storiesCount == 0 || storiesNightModePlaying) {
        return;
    }

    if (storyActionKey[0] == '\0') {
        if (storiesDiplayMode == STORIES_DISPLAY_MODE_SINGLE) {
            stories_setMode(STORIES_DISPLAY_MODE_TILES);
        } else {
            stories_setMode(STORIES_DISPLAY_MODE_SINGLE);
        }
    }
}

void stories_ok(void) {
    if (storiesCount == 0 || storiesNightModePlaying) {
        return;
    }

    if (storyActionKey[0] == '\0') {
        stories_load();
    } else {
        if (stories_nightMode_isPlaylistableAudio()) {
            if (stories_nightMode_addToPlaylist()) {
                stories_nightMode_start();
            } else {
                stories_transition("home");
            }
        } else if (storyOkAction) {
            stories_transition("ok");
        }
    }
}


void stories_autoplay(void) {
    storyOkAction = true;
    stories_ok();
}


void stories_pause(void) {
    if (storiesCount == 0) {
        return;
    }
    if (stories_nightMode_isPlaylistableAudio()) {
        stories_nightMode_addToPlaylist();
        stories_nightMode_start();
    } else {
        if (Mix_PlayingMusic() == 1) {
            if (Mix_PausedMusic() == 1) {
                autosleep_lock();
                Mix_ResumeMusic();
                storyStartTime = get_time();
            } else {
                stories_autosleep_unlock();
                Mix_PauseMusic();
                long int ts = get_time();
                storyTime += (double) (ts - storyStartTime);
                storyStartTime = ts;
            }
        }
    }
}

bool stories_home(void) {
    if (storiesCount == 0) {
        stories_nightMode_reset();
        return true;
    }

    if (storyActionKey[0] == '\0' || storiesNightModePlaying) {
        Mix_HookMusicFinished(NULL);
        if (Mix_PlayingMusic() == 1) {
            if (Mix_PausedMusic() != 1) {
                Mix_PauseMusic();
            }
        }
        stories_nightMode_reset();
        return true;
    } else {
        stories_transition("home");
        return false;
    }
}

void stories_save(void) {
    if (Mix_PlayingMusic() == 1) {
        if (Mix_PausedMusic() != 1) {
            stories_pause();
        }
    }
    stories_saveSession();
}

void stories_reset(void) {
    Mix_HookMusicFinished(NULL);
    storyAutoplay = false;
    storyOkAction = true;
    storyStageKey[0] = '\0';
    storyActionKey[0] = '\0';
    storyActionOptionIndex = 0;
    storyTime = 0;
    if (hasInventory) {
        hasInventory = false;
        free(storyInventoryCount);
        storyInventoryCount = NULL;
        storyInventoryCountLength = -1;
    }
    if (storyJson != NULL) {
        cJSON_free(storyJson);
        storyJson = NULL;
    }
    stories_title();
}

void stories_init(void) {
    if (storiesList != NULL) {
        return stories_title();
    }

    callback_stories_autoplay = &stories_autoplay;
    callback_stories_reset = &stories_reset;
    callback_stories_nightMode = &stories_nightMode_next;

    video_displayImage(SYSTEM_RESOURCES, "loadingStories.png");

    int i = 0;
    DIR *d;
    struct dirent *dir;
    d = opendir(STORIES_RESOURCES);

    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            i++;
        }
    }
    rewinddir(d);

    storiesCount = i;
    char **filesList = (char **) malloc(i * sizeof(char *));
    i = 0;

    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            filesList[i] = (char *) malloc(STR_DIRNAME);
            strcpy(filesList[i], dir->d_name);
            i++;
        }
    }
    closedir(d);

    if (storiesCount > 0) {
        sort(filesList, storiesCount);
    }

    storiesList = filesList;

    if (parameters_getStoryDisplayNine()) {
        storiesDiplayMode = STORIES_DISPLAY_MODE_TILES;
    } else {
        storiesDiplayMode = STORIES_DISPLAY_MODE_SINGLE;
    }

    stories_title();
}

void stories_initNightMode(void) {
    storiesNightModeEnabled = true;
    stories_init();
}

#endif // STORYTELLER_STORIES_HELPER__