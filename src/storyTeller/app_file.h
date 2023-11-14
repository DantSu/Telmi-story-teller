#ifndef STORYTELLER_APP_FILE__
#define STORYTELLER_APP_FILE__

#include <stdio.h>
#include <stdarg.h>

bool file_save(char *filePath, char *format, ...)
{
    FILE *fp;
    if ((fp = fopen(filePath, "w+"))) {
        va_list args;
        va_start(args, format);
        vfprintf(fp, format, args);
        va_end(args);

        fflush(fp);
        fsync(fileno(fp));
        fclose(fp);
        return true;
    }
    return false;
}

#endif // STORYTELLER_APP_FILE__