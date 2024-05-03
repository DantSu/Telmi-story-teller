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

#include "./app_selector.h"

void applock_startTimer(void) {
    appLockTimer = get_time();
    if(appLockIsLocked) {
        app_lockChanged();
    }
}

void applock_stopTimer(void) {
    appLockTimer = 0;
    if(appLockIsLocked) {
        app_lockChanged();
    }
}

void applock_lock(void) {
    applock_stopTimer();
    appLockIsLocked = true;
    appLockChangedTimer = get_time();
    app_lockChanged();
}

void applock_stopLockChangedTimer(void) {
    appLockChangedTimer = 0;
    appLockIsRecentlyUnlocked = false;
}

void applock_unlock(void) {
    appLockIsLocked = false;
    applock_stopTimer();
    appLockIsRecentlyUnlocked = true;
    appLockChangedTimer = get_time();
    app_lockChanged();
}

void applock_checkLock(void) {
    long time = get_time(), laps;

    if (appLockChangedTimer > 0) {
        laps = time - appLockChangedTimer;
        if (laps > 2) {
            applock_stopLockChangedTimer();
            app_lockChanged();
        }
    }

    if (appLockTimer > 0) {
        laps = time - appLockTimer;
        if (laps > 1) {
            if (appLockIsLocked) {
                applock_unlock();
            } else {
                applock_lock();
            }
        }
    }
}

#endif // STORYTELLER_APP_LOCK__