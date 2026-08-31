#ifndef STORYTELLER_TIME_HELPER__
#define STORYTELLER_TIME_HELPER__

#include <time.h>


long int get_time(void) {
    return (long int) time(0);
}


static unsigned long time_lastTime = 0;

bool time_wait300ms(void) {
    unsigned long currentTime = clock() * 1000 / CLOCKS_PER_SEC;
    if ((currentTime - time_lastTime) > 300) {
        time_lastTime = currentTime;
        return true;
    }
    return false;
}


#endif // STORYTELLER_TIME_HELPER__
