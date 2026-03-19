#ifndef PLATFORM_SYSTEM_H__
#define PLATFORM_SYSTEM_H__

#include "utils/flags.h"

// Miyoo Mini shutdown: create .offOrder flag for runtime.sh
static inline void _platform_system_shutdown(void)
{
    // runtime.sh checks for this file to show shutdown screen
    temp_flag_set(".offOrder", true);
}

#define PLATFORM_SYSTEM_SHUTDOWN() _platform_system_shutdown()

#endif // PLATFORM_SYSTEM_H__
