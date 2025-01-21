#ifndef STORYTELLER_LOGS_HELPER__
#define STORYTELLER_LOGS_HELPER__

#include <stdio.h>

void writeLog(char* title, char* message)
{
    printf("[%s.h] : %s\n", title, message);
    fflush(stdout);
}

#endif // STORYTELLER_LOGS_HELPER__