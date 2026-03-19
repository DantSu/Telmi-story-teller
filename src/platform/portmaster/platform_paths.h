#ifndef PLATFORM_PATHS_H__
#define PLATFORM_PATHS_H__

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static bool _platform_paths_initialized = false;
static char _platform_paths_game_dir[PATH_MAX] = {'\0'};
static char _platform_paths_system_resources[PATH_MAX] = {'\0'};
static char _platform_paths_music_resources[PATH_MAX] = {'\0'};
static char _platform_paths_stories_resources[PATH_MAX] = {'\0'};
static char _platform_paths_stories_saves[PATH_MAX] = {'\0'};
static char _platform_paths_savefile[PATH_MAX] = {'\0'};
static char _platform_paths_parameters[PATH_MAX] = {'\0'};
static char _platform_paths_font_regular[PATH_MAX] = {'\0'};
static char _platform_paths_font_bold[PATH_MAX] = {'\0'};

static void _platform_paths_copy(char *dst, const char *src)
{
    strncpy(dst, src, PATH_MAX - 1);
    dst[PATH_MAX - 1] = '\0';
}

static void _platform_paths_join(char *dst, const char *left, const char *right)
{
    _platform_paths_copy(dst, left);
    strncat(dst, right, PATH_MAX - strlen(dst) - 1);
}

static void _platform_paths_init(void)
{
    if (_platform_paths_initialized) {
        return;
    }

    const char *gameDir = getenv("GAMEDIR");
    if (gameDir != NULL && gameDir[0] != '\0') {
        _platform_paths_copy(_platform_paths_game_dir, gameDir);
    }
    else if (getcwd(_platform_paths_game_dir, PATH_MAX) == NULL) {
        _platform_paths_copy(_platform_paths_game_dir, ".");
    }

    _platform_paths_join(_platform_paths_system_resources, _platform_paths_game_dir, "/data/res/");
    _platform_paths_join(_platform_paths_music_resources, _platform_paths_game_dir, "/data/Music/");
    _platform_paths_join(_platform_paths_stories_resources, _platform_paths_game_dir, "/data/Stories/");
    _platform_paths_join(_platform_paths_stories_saves, _platform_paths_game_dir, "/data/Saves/Stories/");
    _platform_paths_join(_platform_paths_savefile, _platform_paths_game_dir, "/data/Saves/.storytellerState");
    _platform_paths_join(_platform_paths_parameters, _platform_paths_game_dir, "/data/Saves/.parameters");
    _platform_paths_join(_platform_paths_font_regular, _platform_paths_system_resources, "Exo2-Regular.ttf");
    _platform_paths_join(_platform_paths_font_bold, _platform_paths_system_resources, "Exo2-Bold.ttf");

    _platform_paths_initialized = true;
}

static const char *_platform_paths_system_resources_get(void)
{
    _platform_paths_init();
    return _platform_paths_system_resources;
}

static const char *_platform_paths_music_resources_get(void)
{
    _platform_paths_init();
    return _platform_paths_music_resources;
}

static const char *_platform_paths_stories_resources_get(void)
{
    _platform_paths_init();
    return _platform_paths_stories_resources;
}

static const char *_platform_paths_stories_saves_get(void)
{
    _platform_paths_init();
    return _platform_paths_stories_saves;
}

static const char *_platform_paths_savefile_get(void)
{
    _platform_paths_init();
    return _platform_paths_savefile;
}

static const char *_platform_paths_parameters_get(void)
{
    _platform_paths_init();
    return _platform_paths_parameters;
}

static const char *_platform_paths_font_regular_get(void)
{
    _platform_paths_init();
    return _platform_paths_font_regular;
}

static const char *_platform_paths_font_bold_get(void)
{
    _platform_paths_init();
    return _platform_paths_font_bold;
}

#define SYSTEM_RESOURCES      ((char *)_platform_paths_system_resources_get())
#define MUSICPLAYER_RESOURCES ((char *)_platform_paths_music_resources_get())
#define STORIES_RESOURCES     ((char *)_platform_paths_stories_resources_get())
#define STORIES_SAVES         ((char *)_platform_paths_stories_saves_get())
#define APP_SAVEFILE          ((char *)_platform_paths_savefile_get())
#define APP_PARAMETERS_PATH   ((char *)_platform_paths_parameters_get())
#define FALLBACK_FONT_REGULAR ((char *)_platform_paths_font_regular_get())
#define FALLBACK_FONT_BOLD    ((char *)_platform_paths_font_bold_get())

#endif // PLATFORM_PATHS_H__
