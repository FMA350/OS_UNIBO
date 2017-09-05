#ifndef ACCOUNTING_H
#define ACCOUNTING_H

#include <mikabooq.h>
#include "stopwatch.h"

// defined in accounting.c
extern unsigned int timeSliceLeft;
extern unsigned int TICKS_PER_TIME_SLICE;   // initialized in init.c
extern unsigned int pseudoclock;
extern struct list_head t_wait4clock;
extern stopwatch_t sys_stopwatch;

unsigned int accountant(struct tcb_t* thread);
void update_clock(unsigned int milliseconds);

#endif
