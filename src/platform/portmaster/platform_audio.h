#ifndef PLATFORM_AUDIO_H__
#define PLATFORM_AUDIO_H__

#include <stdbool.h>

#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"

typedef double (SDLCALL *PlatformMixMusicDurationFn)(Mix_Music *music);
typedef double (SDLCALL *PlatformMixGetMusicPositionFn)(Mix_Music *music);

typedef struct {
    void *mixer_handle;
    PlatformMixMusicDurationFn music_duration_fn;
    PlatformMixGetMusicPositionFn get_music_position_fn;
    double tracked_music_position;
    Uint32 tracked_music_position_tick;
    bool tracked_music_position_advancing;
} PlatformAudioState;

static inline void platform_audio_init(PlatformAudioState *state)
{
    state->mixer_handle = SDL_LoadObject("libSDL2_mixer-2.0.so.0");
    if (state->mixer_handle == NULL) {
        state->mixer_handle = SDL_LoadObject("libSDL2_mixer.so.0");
    }
    if (state->mixer_handle == NULL) {
        state->mixer_handle = SDL_LoadObject("libSDL2_mixer.so");
    }

    state->music_duration_fn = NULL;
    state->get_music_position_fn = NULL;
    if (state->mixer_handle != NULL) {
        state->music_duration_fn = (PlatformMixMusicDurationFn) SDL_LoadFunction(state->mixer_handle, "Mix_MusicDuration");
        state->get_music_position_fn = (PlatformMixGetMusicPositionFn) SDL_LoadFunction(state->mixer_handle, "Mix_GetMusicPosition");
    }

    state->tracked_music_position = 0.0;
    state->tracked_music_position_tick = 0;
    state->tracked_music_position_advancing = false;
}

static inline void platform_audio_quit(PlatformAudioState *state)
{
    if (state->mixer_handle != NULL) {
        SDL_UnloadObject(state->mixer_handle);
        state->mixer_handle = NULL;
    }

    state->music_duration_fn = NULL;
    state->get_music_position_fn = NULL;
    state->tracked_music_position = 0.0;
    state->tracked_music_position_tick = 0;
    state->tracked_music_position_advancing = false;
}

static inline bool platform_audio_can_query_duration(const PlatformAudioState *state)
{
    return state->music_duration_fn != NULL;
}

static inline double platform_audio_query_duration(PlatformAudioState *state, Mix_Music *music)
{
    if (state->music_duration_fn == NULL) {
        return -1.0;
    }

    return state->music_duration_fn(music);
}

static inline void platform_audio_forget_music(PlatformAudioState *state)
{
    state->tracked_music_position = 0.0;
    state->tracked_music_position_tick = 0;
    state->tracked_music_position_advancing = false;
}

static inline void platform_audio_track_position_update(PlatformAudioState *state)
{
    if (!state->tracked_music_position_advancing) {
        return;
    }

    Uint32 now = SDL_GetTicks();
    state->tracked_music_position += (double) (now - state->tracked_music_position_tick) / 1000.0;
    state->tracked_music_position_tick = now;
}

static inline void platform_audio_track_position_start(PlatformAudioState *state, double position)
{
    state->tracked_music_position = position;
    state->tracked_music_position_tick = SDL_GetTicks();
    state->tracked_music_position_advancing = true;
}

static inline void platform_audio_track_position_sync(PlatformAudioState *state, Mix_Music *music)
{
    if (music == NULL || Mix_PlayingMusic() == 0) {
        platform_audio_track_position_update(state);
        state->tracked_music_position_advancing = false;
        return;
    }

    if (Mix_PausedMusic() == 1) {
        platform_audio_track_position_update(state);
        state->tracked_music_position_advancing = false;
        return;
    }

    if (!state->tracked_music_position_advancing) {
        state->tracked_music_position_tick = SDL_GetTicks();
        state->tracked_music_position_advancing = true;
    }
}

static inline void platform_audio_seek(PlatformAudioState *state, Mix_Music *music, double position)
{
    (void) music;
    Mix_SetMusicPosition(position);

    if (Mix_PausedMusic() == 1) {
        state->tracked_music_position = position;
        state->tracked_music_position_tick = SDL_GetTicks();
        state->tracked_music_position_advancing = false;
        return;
    }

    platform_audio_track_position_start(state, position);
}

static inline double platform_audio_get_position(PlatformAudioState *state, Mix_Music *music)
{
    if (music == NULL) {
        return 0.0;
    }

    if (state->get_music_position_fn != NULL) {
        double position = state->get_music_position_fn(music);
        if (position >= 0.0) {
            return position;
        }
    }

    platform_audio_track_position_sync(state, music);
    if (!state->tracked_music_position_advancing) {
        return state->tracked_music_position;
    }

    return state->tracked_music_position + (double) (SDL_GetTicks() - state->tracked_music_position_tick) / 1000.0;
}

#endif // PLATFORM_AUDIO_H__
