#ifndef SCHEDULER_H
#define SCHEDULER_H

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


#endif
