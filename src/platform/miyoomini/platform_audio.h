#ifndef PLATFORM_AUDIO_H__
#define PLATFORM_AUDIO_H__

#include <stdbool.h>

#include "SDL2/SDL_mixer.h"

typedef struct {
    int unused;
} PlatformAudioState;

static inline void platform_audio_init(PlatformAudioState *state)
{
    (void) state;
}

static inline void platform_audio_quit(PlatformAudioState *state)
{
    (void) state;
}

static inline bool platform_audio_can_query_duration(const PlatformAudioState *state)
{
    (void) state;
    return true;
}

static inline double platform_audio_query_duration(PlatformAudioState *state, Mix_Music *music)
{
    (void) state;
    return Mix_MusicDuration(music);
}

static inline void platform_audio_forget_music(PlatformAudioState *state)
{
    (void) state;
}

static inline void platform_audio_seek(PlatformAudioState *state, Mix_Music *music, double position)
{
    (void) state;
    (void) music;
    Mix_SetMusicPosition(position);
}

static inline double platform_audio_get_position(PlatformAudioState *state, Mix_Music *music)
{
    (void) state;
    return Mix_GetMusicPosition(music);
}

#endif // PLATFORM_AUDIO_H__
