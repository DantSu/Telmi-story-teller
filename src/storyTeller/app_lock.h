#ifndef STORYTELLER_APP_LOCK__
#define STORYTELLER_APP_LOCK__

#include "./time_helper.h"

static bool app_lock_IsLocked = false;
static bool app_lock_IsRecentlyUnlocked = false;
static long app_lock_Timer = 0;
static long app_lock_ChangedTimer = 0;

bool applock_isLocked(void) {
    return app_lock_IsLocked;
}

bool applock_isRecentlyUnlocked(void) {
    return app_lock_IsRecentlyUnlocked;
}

bool applock_isLockRecentlyChanged(void) {
    return app_lock_ChangedTimer > 0;
}

bool applock_isUnlocking(void) {
    return app_lock_Timer > 0 && app_lock_IsLocked;
}

bool applock_startTimer(void) {
    app_lock_Timer = get_time();
    return app_lock_IsLocked;
}

bool applock_stopTimer(void) {
    app_lock_Timer = 0;
    return app_lock_IsLocked;
}

bool applock_lock(void) {
    applock_stopTimer();
    app_lock_IsLocked = true;
    app_lock_ChangedTimer = get_time();
    return true;
}

void applock_stopLockChangedTimer(void) {
    app_lock_ChangedTimer = 0;
    app_lock_IsRecentlyUnlocked = false;
}

bool applock_unlock(void) {
    app_lock_IsLocked = false;
    applock_stopTimer();
    app_lock_IsRecentlyUnlocked = true;
    app_lock_ChangedTimer = get_time();
    return true;
}

bool applock_checkLock(void) {
    long time = get_time(), laps;

    if (app_lock_ChangedTimer > 0) {
        laps = time - app_lock_ChangedTimer;
        if (laps > 2) {
            applock_stopLockChangedTimer();
            return true;
        }
    }

    if (app_lock_Timer > 0) {
        laps = time - app_lock_Timer;
        if (laps > 1) {
            if (app_lock_IsLocked) {
                return applock_unlock();
            } else {
                return applock_lock();
            }
        }
    }
    return false;
}

#endif // STORYTELLER_APP_LOCK__