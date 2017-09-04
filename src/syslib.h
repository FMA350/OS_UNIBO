#ifndef SYSLIB_H
#define SYSLIB_H

#include <sys/types.h>
#include <libuarm.h>

#include "scheduler.h"

void *memcpy(void *dest, const void *source, size_t num);


/* tprint with formatted output
 *
 * function with undetermined number of arguments.
 * Preconditions: format != NULL
 * Postconditions: Returns the number of characters printed.
 */
int tprintf(const char *format, ...);

void BREAKPOINT();


#define DEBUG   1

static inline void assert(int assertion)
{
    #if DEBUG
    if (!(assertion)) {
        tprintf("Assertion failed\n"
                "current_thread == %p\n",
                current_thread);
        PANIC();
    }
    #endif // DEBUG
}

#endif // SYSLIB_H
