#ifndef PLATFORM_SYSTEM_H__
#define PLATFORM_SYSTEM_H__

#include "utils/flags.h"

// PortMaster shutdown: create flag file for launcher script to detect
static inline void _platform_system_shutdown(void)
{
    // The launcher script checks for this file at exit
    temp_flag_set("telmi_poweroff.flag", true);
}

#define PLATFORM_SYSTEM_SHUTDOWN() _platform_system_shutdown()

#endif // PLATFORM_SYSTEM_H__
