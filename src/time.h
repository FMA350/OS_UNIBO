#ifndef TIME_H
#define TIME_H

#include <mikabooq.h>
#include "stopwatch.h"

// defined in accounting.c
extern struct list_head t_wait4clock;
extern unsigned int timeSliceLeft;
extern unsigned int cyclesUsed;
extern uint32_t TICKS_PER_TIME_SLICE;
extern stopwatch_t sys_stopwatch;


void time_init(void);


#endif
