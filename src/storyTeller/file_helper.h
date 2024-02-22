#ifndef STORYTELLER_FILE_HELPER__
#define STORYTELLER_FILE_HELPER__

#include <sys/stat.h>

long file_getSize(char* filePath)
{
    struct stat st; 
    if (stat(filePath, &st) == 0) {
        return (long)st.st_size;
    }
    return -1; 
}



#endif // STORYTELLER_FILE_HELPER__