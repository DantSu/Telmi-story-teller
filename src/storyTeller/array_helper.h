#ifndef STORYTELLER_ARRAY_HELPER__
#define STORYTELLER_ARRAY_HELPER__



static int compareAlpha(const void* a, const void* b)
{
    return strcmp(*(const char**)a, *(const char**)b);
}

void sort(char** arr, int n)
{
    qsort(arr, n, sizeof(const char*), compareAlpha);
}



#endif // STORYTELLER_ARRAY_HELPER__