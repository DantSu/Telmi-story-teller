#ifndef STORYTELLER_APP_AUTOSLEEP__
#define STORYTELLER_APP_AUTOSLEEP__

#include <time.h>

#include "system/display.h"

#include "./app_parameters.h"

static bool autosleepLocked = false;
static long int autosleepTime = 0;
static int autosleepTimeScreenOn = 0;
static int autosleepTimeScreenOff = 0;

long int autosleep_timestamp(void) {
    return (long int) time(0);
}

void autosleep_keepAwake(void)
{
    if(display_enabled) {
        autosleepTime = autosleep_timestamp() + autosleepTimeScreenOn;
    } else {
        autosleepTime = autosleep_timestamp() + autosleepTimeScreenOff;
    }
}

bool autosleep_isSleepingTime(void)
{
    long int cTime = autosleep_timestamp();
    if(!autosleepLocked && cTime > autosleepTime) {
        return true;
    }
    return false;
}

void autosleep_lock(void) {
    autosleepLocked = true;
}

void autosleep_unlock(int timeScreenOn, int timeScreenOff) {
    autosleepTimeScreenOn = timeScreenOn;
    autosleepTimeScreenOff = timeScreenOff;
    autosleep_keepAwake();
    autosleepLocked = false;
}

void autosleep_init(int timeScreenOn, int timeScreenOff)
{
    autosleep_unlock(timeScreenOn, timeScreenOff);
}

#endif // STORYTELLER_APP_AUTOSLEEP__