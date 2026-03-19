#ifndef PLATFORM_BATTERY_H__
#define PLATFORM_BATTERY_H__

#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include "utils/file.h"
#include "utils/log.h"
#include "utils/msleep.h"
#include "utils/process.h"
#include "system/device_model.h"

// Miyoo Mini battery is managed by batmon process writing to /tmp/percBat
static int _platform_battery_get_percentage(void)
{
    FILE *fp;
    int percentage = -1;
    int retry = 3;

    while (percentage == -1 && retry > 0) {
        if (exists("/tmp/percBat")) {
            file_get(fp, "/tmp/percBat", "%d", &percentage);
            break;
        }
        else {
            printf_debug("/tmp/percBat not found (%d)\n", retry);

            if (!process_isRunning("batmon")) {
                printf_debug("bin/batmon not running (%d)\n", retry);
                break;
            }
        }
        retry--;
        msleep(100);
    }

#ifdef LOG_DEBUG
    return 78;
#endif

    if (percentage == -1)
        percentage = 0; // show zero when percBat not found

    return percentage;
}

static bool _platform_battery_is_charging(void)
{
    if (DEVICE_ID == MIYOO283) {
        char charging = 0;
        int fd = open(GPIO_DIR2 "gpio59/value", O_RDONLY);

        if (fd < 0) {
            // export gpio59, direction: in
            file_write(GPIO_DIR1 "export", "59", 2);
            file_write(GPIO_DIR2 "gpio59/direction", "in", 2);
            fd = open(GPIO_DIR2 "gpio59/value", O_RDONLY);
        }

        if (fd >= 0) {
            read(fd, &charging, 1);
            close(fd);
        }

        return charging == '1';
    }
    else if (DEVICE_ID == MIYOO354) {
        char *cmd = "cd /customer/app/ ; ./axp_test";
        int batJsonSize = 100;
        char buf[batJsonSize];
        int charge_number;

        FILE *fp;
        fp = popen(cmd, "r");
        if (fgets(buf, batJsonSize, fp) != NULL) {
            sscanf(buf, "{\"battery\":%*d, \"voltage\":%*d, \"charging\":%d}",
                   &charge_number);
        }
        pclose(fp);
        return charge_number == 3;
    }
    return false;
}

#define PLATFORM_BATTERY_GET_PERCENTAGE() _platform_battery_get_percentage()
#define PLATFORM_BATTERY_IS_CHARGING()    _platform_battery_is_charging()

#endif // PLATFORM_BATTERY_H__
