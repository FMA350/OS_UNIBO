#include <mikabooq.h>
#include <nucleus.h>
#include <arch.h>
#include <scheduler.h>
#include <handlers.h>
#include <math.h>

#include "syslib.h"
#include "time.h"

#include <pseudoclock.h>
#include <stopwatch.h>

unsigned int timeSliceLeft;
unsigned int cyclesUsed;

uint32_t TICKS_PER_TIME_SLICE;
stopwatch_t sys_stopwatch;


void time_init(void) {
    TICKS_PER_TIME_SLICE = *((unsigned int *) (BUS_REG_TIME_SCALE)) * 5000;
    pseudoclock_init();
    stopwatch_init(&sys_stopwatch);
}
