#ifndef SCHEDULER_H
#define SCHEDULER_H

extern struct tcb_t *current_thread;
extern struct list_head readyq;

extern unsigned int thread_count;
extern unsigned int soft_block_count;


void load_readyq(struct pcb_t *root);

void experimentalClerk();

void scheduler();


#endif
