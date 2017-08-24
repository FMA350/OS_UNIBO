#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

extern struct tcb_t *current_thread;
extern struct list_head readyq;
extern struct list_head blockedq;

extern unsigned int thread_count;
extern unsigned int soft_block_count;

extern unsigned int clockPerTimeslice;

/*
 * Preconditions:
 *  This function should only be called during initialization.
 *  to_load is the tcb_t to be loaded in the ready queue, target is the function
 *  the thread will execute, cpsr is the starting value of cpsr register for the
 *  thread.
 *
 * Postconditions:
 *  Initializes the thread and puts it into the ready queue.
 *  Global thread counter is incremented.
 */
extern void init_and_load(struct tcb_t *to_load, void *target, unsigned int cpsr);


void load_readyq(struct pcb_t *root);

unsigned int accountant(struct tcb_t* thread);

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
