#ifndef STORYTELLER_APP_AUTOSLEEP__
#define STORYTELLER_APP_AUTOSLEEP__

#include <time.h>

#include "system/display.h"

#define AUTOSLEEP_INACTIVE_TIME_SCREEN_ON 30
#define AUTOSLEEP_INACTIVE_TIME_SCREEN_OFF 180

static bool autosleepLocked = false;
static long int autosleepTime = 0;

long int autosleep_timestamp(void) {
    return (long int) time(0);
}

void autosleep_keepAwake(void)
{
    if(display_enabled) {
        autosleepTime = autosleep_timestamp() + AUTOSLEEP_INACTIVE_TIME_SCREEN_ON;
    } else {
        autosleepTime = autosleep_timestamp() + AUTOSLEEP_INACTIVE_TIME_SCREEN_OFF;
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

void autosleep_lock() {
    autosleepLocked = true;
}

void autosleep_unlock() {
    autosleep_keepAwake();
    autosleepLocked = false;
}

void autosleep_init(void)
{
    autosleep_keepAwake();
}

#endif // STORYTELLER_APP_AUTOSLEEP__