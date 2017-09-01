#ifndef DO_IO_S_H
#define DO_IO_S_H

#include <arch.h>

#define RECV_STATUS     0x0
#define RECV_COMMAND    0x4
#define TRANSM_STATUS   0x8
#define TRANSM_COMMAND  0xC

#define TERMINAL_DEV_FIELD(dev, field) (DEV_REG_ADDR(IL_TERMINAL, dev) + (field))


struct io_req {
    uintptr_t val;
    struct tcb_t *requester;
};

// la dichiarazione per ora è in ssi.c
extern struct io_req request[5];

#endif // DO_IO_S_H
