#ifndef SSI_H
#define SSI_H

#include <mikabooq.h>


/* SSI daemon */
// void SSI();

// FIXME: should errorNumber be moved in ssi.c and called static???
int errorNumber;
// TODO: is error number different from thread to thread???

/* Returns the ssi thread already initialized, extept for processor state */
struct tcb_t *ssi_thread_init();

/* SERVICES */
int GET_ERRNO();

struct pcb_t *CREATE_PROCESS(state_t initial_state);

struct tcb_t *CREATE_THREAD(state_t initial_state, struct pcb_t *process);

int TERMINATE_PROCESS(struct pcb_t *processToDelete);

int TERMINATE_THREAD(struct tcb_t *threadToDelete);

unsigned int GETCPUTIME(struct tcb_t * thread);

struct pcb_t *GET_PROCESSID(struct tcb_t * thread);

#endif
