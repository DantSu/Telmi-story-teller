#ifndef STORYTELLER_APP_LOCK__
#define STORYTELLER_APP_LOCK__

#include "./time_helper.h"
#include "system/display.h"

static bool applockIsLocked = false;

bool applock_isLocked(void)
{
    return applockIsLocked;
}

#include "./sdl_helper.h"


static long applockTimer = 0;
static long applockTimerScreenOn = 0;

void applock_startScreenTimer(void)
{
    applockTimerScreenOn = get_time();
    display_setScreen(true);
}

void applock_stopScreenTimer(void)
{
    applockTimerScreenOn = 0;
}

void applock_endScreenTimer(void)
{
    applock_stopScreenTimer();
    display_setScreen(false);
}

void applock_startTimer(void)
{
    applockTimer = get_time();
    if(applockIsLocked && !display_enabled) {
        applock_startScreenTimer();
    }
}

void applock_stopTimer(void)
{
    applockTimer = 0;
}

void applock_lock(void)
{
    applock_stopTimer();
    applockIsLocked = true;
    video_applyToVideo();
}

void applock_unlock(void)
{
    applock_stopTimer();
    applockIsLocked = false;
    video_applyToVideo();
}

void applock_checkLock(void)
{
    long time = get_time(), laps;

    if(applockTimerScreenOn > 0) {
        laps = time - applockTimerScreenOn;
        if(laps > 3) {
            applock_endScreenTimer();
        }
    }

    if(applockTimer == 0) {
        return;
    }

    if(applockIsLocked && !display_enabled && applockTimerScreenOn == 0) {
        applock_startScreenTimer();
    }

    laps = time - applockTimer;

    if(laps > 2) {
        if(applockIsLocked) {
            applock_unlock();
        } else {
            applock_lock();
            if(!display_enabled) {
                applock_startScreenTimer();
            }
        }
    }
}

#endif // STORYTELLER_APP_LOCK__