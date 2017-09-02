#ifndef DO_IO_H
#define DO_IO_H

#include <arch.h>

#define RECV_STATUS     0x0
#define RECV_COMMAND    0x4
#define TRANSM_STATUS   0x8
#define TRANSM_COMMAND  0xC

#define TERMINAL_DEV_FIELD(dev, field) (DEV_REG_ADDR(IL_TERMINAL, dev) + (field))



// la dichiarazione per ora è in ssi.c
extern struct tcb_t *soft_blocked_thread[5];

#define TERMINAL_REQUESTER_INDEX    4

#endif // DO_IO_S_H
