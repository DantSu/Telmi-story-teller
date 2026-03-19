#ifndef BATTERY_H__
#define BATTERY_H__

#include "system/device_model.h"
#include "system/system.h"
#include "utils/file.h"
#include "utils/log.h"
#include "utils/msleep.h"
#include "utils/process.h"

#include "platform_battery.h"

static bool battery_is_charging = false;

/**
 * @brief Retrieve the current battery percentage as reported by batmon
 *
 * @return int : Battery percentage (0-100) or 500 if charging
 */

int battery_getPercentage(void)
{
    return PLATFORM_BATTERY_GET_PERCENTAGE();
}

bool battery_isCharging(void)
{
    return PLATFORM_BATTERY_IS_CHARGING();
}

bool battery_hasChanged(int ticks, int *out_percentage)
{
    bool changed = false;

    if (battery_isCharging()) {
        if (!battery_is_charging) {
            *out_percentage = 500;
            battery_is_charging = true;
            return true;
        }
        return false;
    }
    else if (battery_is_charging) {
        battery_is_charging = false;
    }

    // Platform-agnostic: just poll the battery percentage
    int current_percentage = battery_getPercentage();

    if (current_percentage != *out_percentage) {
        *out_percentage = current_percentage;
        changed = true;
    }

    return changed;
}

#endif // BATTERY_H__
