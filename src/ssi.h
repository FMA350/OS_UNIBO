#ifndef SSI_H
#define SSI_H

#include <mikabooq.h>

/* SSI's thread defined in ssi.c */
extern struct tcb_t *SSI;

/* SSI target function */
void ssi(void);

/* Returns a pointer to the ssi thread already initialized,
   extept for processor state */
struct tcb_t *ssi_thread_init(void);

#endif
