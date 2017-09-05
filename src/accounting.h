#ifndef ACCOUNTING_H
#define ACCOUNTING_H

#include <mikabooq.h>

// defined in accounting.c
extern unsigned int timeSliceLeft;
extern unsigned int TICKS_PER_TIME_SLICE;   // initialized in init.c
extern unsigned int pseudoclock;
extern struct list_head t_wait4clock;

unsigned int accountant(struct tcb_t* thread);
void update_clock(unsigned int milliseconds);


static inline uint64_t pack(const uint32_t RegLow, const uint32_t RegHigh) {
    uint64_t rval = RegHigh;
    return (rval << 32) | RegLow;
}

/* Unpacks Reg in RegLow and RegHigh*/
static inline unpack(const uint64_t Reg, uint32_t *RegLow, uint32_t *RegHigh) {
    *RegLow = (uint32_t) Reg;
    *RegHigh = (uint32_t) (Reg >> 32);
}

#endif
