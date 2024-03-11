#ifndef STORYTELLER_TIME_HELPER__
#define STORYTELLER_TIME_HELPER__

#include <time.h>


static long int get_time(void)
{
    return (long int) time(0);
}



#endif // STORYTELLER_TIME_HELPER__