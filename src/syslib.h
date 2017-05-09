#ifndef SYSLIB_H
#define SYSLIB_H

#include <sys/types.h>


void *memcpy(void *dest, const void *source, size_t num);


/* tprint with formatted output
 *
 * function with undetermined number of arguments.
 * Preconditions: format != NULL
 * Postconditions: Returns the number of characters printed.
 */
int tprintf(const char *format, ...);


#endif
