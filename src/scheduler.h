#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

extern struct tcb_t *current_thread;
extern struct list_head readyq;

extern unsigned int thread_count;
extern unsigned int soft_block_count;

extern unsigned int clockPerTimeslice;




void load_readyq(struct pcb_t *root);

void experimentalClerk();

/* used also in debug */
inline void init_and_load(struct tcb_t *to_load, void *target, unsigned int status);

void scheduler();


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
