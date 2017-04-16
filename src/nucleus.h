#ifndef NUCLEUS_H
#define NUCLEUS_H

/* Syscall mnemonic values */
#define SYS_ERR     0
#define SYS_SEND 	1
#define SYS_RECV	2

/* SSI requests */
#define GET_ERRNO 0
#define CREATE_PROCESS 1
#define CREATE_THREAD 2
#define TERMINATE_PROCESS 3
#define TERMINATE_THREAD 4
#define SETPGMMGR 5
#define SETTLBMGR 6
#define SETSYSMGR 7
#define GETCPUTIME 8
#define WAIT_FOR_CLOCK 9
#define DO_IO 10
#define GET_PROCESSID 11
#define GET_THREAD 12

extern void *SSI;

#define msgsend(dest,payload) (SYSCALL(SYS_SEND,(unsigned int) (dest),(unsigned int) (payload),0))

/* if source == NULL si prende un messaggio da qualunque sender */
#define msgrecv(source,reply) (((struct tcb_t *) SYSCALL(SYS_RECV,(unsigned int) (source),(unsigned int) (reply),0)))

#endif
