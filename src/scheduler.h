#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <mikabooq.h>

// defined in scheduler.c
extern struct tcb_t *current_thread;
extern struct list_head readyq;
extern struct list_head blockedq;

extern unsigned int thread_count;
extern unsigned int soft_block_count;

extern int is_idle;
extern int clockPerTimeslice;

void scheduler(void);

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
/* used also in debug */

// used in init.c during initialization
void load_readyq(struct pcb_t *root);

#endif
