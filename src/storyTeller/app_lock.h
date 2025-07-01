#ifndef STORYTELLER_APP_LOCK__
#define STORYTELLER_APP_LOCK__

#include "./time_helper.h"

static bool appLockIsLocked = false;
static bool appLockIsRecentlyUnlocked = false;
static long appLockTimer = 0;
static long appLockChangedTimer = 0;

bool applock_isLocked(void) {
    return appLockIsLocked;
}

bool applock_isRecentlyUnlocked(void) {
    return appLockIsRecentlyUnlocked;
}

bool applock_isLockRecentlyChanged(void) {
    return appLockChangedTimer > 0;
}

bool applock_isUnlocking(void) {
    return appLockTimer > 0 && appLockIsLocked;
}

bool applock_startTimer(void) {
    appLockTimer = get_time();
    return appLockIsLocked;
}

bool applock_stopTimer(void) {
    appLockTimer = 0;
    return appLockIsLocked;
}

bool applock_lock(void) {
    applock_stopTimer();
    appLockIsLocked = true;
    appLockChangedTimer = get_time();
    return true;
}

void applock_stopLockChangedTimer(void) {
    appLockChangedTimer = 0;
    appLockIsRecentlyUnlocked = false;
}

bool applock_unlock(void) {
    appLockIsLocked = false;
    applock_stopTimer();
    appLockIsRecentlyUnlocked = true;
    appLockChangedTimer = get_time();
    return true;
}

bool applock_checkLock(void) {
    long time = get_time(), laps;

    if (appLockChangedTimer > 0) {
        laps = time - appLockChangedTimer;
        if (laps > 2) {
            applock_stopLockChangedTimer();
            return true;
        }
    }

    if (appLockTimer > 0) {
        laps = time - appLockTimer;
        if (laps > 1) {
            if (appLockIsLocked) {
                return applock_unlock();
            } else {
                return applock_lock();
            }
        }
    }
    return false;
}

#endif // STORYTELLER_APP_LOCK__