#ifndef STORYTELLER_APP_AUTOSLEEP__
#define STORYTELLER_APP_AUTOSLEEP__

#include <time.h>

#include "system/display.h"

#include "./app_parameters.h"

static bool app_autosleep_locked = false;
static long int app_autosleep_time = 0;
static int app_autosleep_timeScreenOn = 0;
static int app_autosleep_timeScreenOff = 0;

long int autosleep_timestamp(void) {
    return (long int) time(0);
}

void autosleep_keepAwake(void)
{
    if(display_enabled) {
        app_autosleep_time = autosleep_timestamp() + app_autosleep_timeScreenOn;
    } else {
        app_autosleep_time = autosleep_timestamp() + app_autosleep_timeScreenOff;
    }
}

bool autosleep_isSleepingTime(void)
{
    long int cTime = autosleep_timestamp();
    if(!app_autosleep_locked && cTime > app_autosleep_time) {
        return true;
    }
    return false;
}

void autosleep_lock(void) {
    app_autosleep_locked = true;
}

void autosleep_unlock(int timeScreenOn, int timeScreenOff) {
    app_autosleep_timeScreenOn = timeScreenOn;
    app_autosleep_timeScreenOff = timeScreenOff;
    autosleep_keepAwake();
    app_autosleep_locked = false;
}

void autosleep_init(int timeScreenOn, int timeScreenOff)
{
    autosleep_unlock(timeScreenOn, timeScreenOff);
}

#endif // STORYTELLER_APP_AUTOSLEEP__