#ifndef STORYTELLER_BATTERY__
#define STORYTELLER_BATTERY__

#include "system/battery.h"
#include "./time_helper.h"

static int app_battery_percentage = 0;
static long int app_battery_percentageTime = 0;

static int app_battery_getPercentage(void)
{
    long int time = get_time();
    if(app_battery_percentageTime < time) {
        app_battery_percentageTime = time + 10;
        app_battery_percentage = battery_getPercentage();
    }
    return app_battery_percentage;
}

#endif // STORYTELLER_BATTERY__