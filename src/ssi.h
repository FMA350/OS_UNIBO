#ifndef SSI_H
#define SSI_H

#include <mikabooq.h>


/* SSI daemon */
// void SSI();

// FIXME: should errorNumber belong to the struct tcb_t???
int errorNumber;
// TODO: is error number different from thread to thread???

/* Returns the ssi thread already initialized, extept for processor state */
struct tcb_t *ssi_thread_init();



#endif
