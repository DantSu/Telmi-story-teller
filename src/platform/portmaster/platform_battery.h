#ifndef PLATFORM_BATTERY_H__
#define PLATFORM_BATTERY_H__

#include <stdbool.h>
#include <stdio.h>
#include "utils/file.h"

// Linux power supply sysfs paths
// Common paths for ARM handheld devices (R36S, RK3326, etc.)
#define BATTERY_CAPACITY_PATH "/sys/class/power_supply/battery/capacity"
#define BATTERY_STATUS_PATH   "/sys/class/power_supply/battery/status"

// Alternative paths for some devices
#define BATTERY_CAPACITY_ALT  "/sys/class/power_supply/BAT0/capacity"
#define BATTERY_STATUS_ALT    "/sys/class/power_supply/BAT0/status"

static int _platform_battery_get_percentage(void)
{
    FILE *fp;
    int percentage = 0;

    // Try primary battery path
    if (exists(BATTERY_CAPACITY_PATH)) {
        file_get(fp, BATTERY_CAPACITY_PATH, "%d", &percentage);
    }
    // Try alternative path
    else if (exists(BATTERY_CAPACITY_ALT)) {
        file_get(fp, BATTERY_CAPACITY_ALT, "%d", &percentage);
    }

    // Clamp to valid range
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;

    return percentage;
}

static bool _platform_battery_is_charging(void)
{
    FILE *fp;
    char status[16] = {0};

    // Try primary battery path
    if (exists(BATTERY_STATUS_PATH)) {
        fp = fopen(BATTERY_STATUS_PATH, "r");
        if (fp) {
            if (fgets(status, sizeof(status), fp) != NULL) {
                fclose(fp);
                // Status is "Charging" or "Discharging" or "Full" or "Not charging"
                return (strncmp(status, "Charging", 8) == 0);
            }
            fclose(fp);
        }
    }
    // Try alternative path
    else if (exists(BATTERY_STATUS_ALT)) {
        fp = fopen(BATTERY_STATUS_ALT, "r");
        if (fp) {
            if (fgets(status, sizeof(status), fp) != NULL) {
                fclose(fp);
                return (strncmp(status, "Charging", 8) == 0);
            }
            fclose(fp);
        }
    }

    return false;
}

#define PLATFORM_BATTERY_GET_PERCENTAGE() _platform_battery_get_percentage()
#define PLATFORM_BATTERY_IS_CHARGING()    _platform_battery_is_charging()

#endif // PLATFORM_BATTERY_H__
