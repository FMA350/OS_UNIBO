#include <syslib.h>


// TODO: Optimize memcpy
void *memcpy(void *dest, const void *source, size_t num) {
    int i = 0;
    // casting pointers
    char *dest8 = (char *)dest;
    char *source8 = (char *)source;
    for (i = 0; i < num; i++)
        dest8[i] = source8[i];
    return dest;
}
