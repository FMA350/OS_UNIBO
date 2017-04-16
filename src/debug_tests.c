#include <libuarm.h>
#include <sys/types.h>
#include <nucleus.h>
#include <arch.h>
#include <debug_tests.h>

extern void BREAKPOINT();
extern unsigned int thread_count;

void test_wait() {
    tprint(" The test has started\n"
           " Ready to wait\n");
    BREAKPOINT();
    WAIT();
}

void test_syscall() {
    tprint("test_syscall has started\n");
    BREAKPOINT();
    tprint("First syscall...\n"
           "All arguments are 1\n");
    SYSCALL(1, 1, 1, 1);
    tprint("Second syscall...\n"
           "All arguments are 0\n");
    SYSCALL(0, 0, 0, 0);
    tprint("Third syscall...\n"
           "First two args are 1 the others are 0\n");
    SYSCALL(1, 1, 0, 0);
    HALT();
}

void test_timer() {
    tprint("test_timer started\n");
    register size_t i, j;
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 100; j++) {
            WAIT();
        }
        BREAKPOINT();
        tprint("100 done\n");
    }
    tprint("terminating test_timer\n");
}

/*********************************************************************************/

void test_rval_msgsend(int rval) {
    if (rval == 0)
        tprint("msgsend succeded\n");
    else if (rval == -1)
        tprint("msgsend failed\n");
}

void test_fail_msg() {
    tprint("test_fail_msg started\n");

    int rval = msgsend(NULL, 1);

    BREAKPOINT();

    test_rval_msgsend(rval);

    HALT();
}

struct tcb_t *sender;
struct tcb_t *receiver;

extern struct list_head readyq;

void test_succed_msg_recv() {
    tprint("test_succed_msg_recv started\n");
    uintptr_t store;
    struct tcb_t *sender_of_msg = msgrecv(NULL, &store);

    if (store == 1)
        tprint("Right value received!\n");
    else
        tprint("Wrong value received!\n");

    tprint("test_succed_msg_recv terminated\n");
    // HALT();
    msgrecv(NULL, &store);
}


void test_succed_msg_send() {
    tprint("test_succed_msg_send started\n");
    int rval = msgsend(receiver, 1);
    test_rval_msgsend(rval);

    // stop the process
    uintptr_t store;
    msgrecv(NULL, &store);
}


void test_succed_msg_init(struct pcb_t *root) {
    tprint("test_succed_msg_init started\n");

    sender = thread_alloc(root);
    receiver = thread_alloc(root);


    receiver->t_s.pc = (unsigned int) test_succed_msg_recv;
    // SP
    receiver->t_s.sp = RAM_TOP - FRAME_SIZE;
    // CPSR -> mask all interrupts and be in kernel mode
    receiver->t_s.cpsr = STATUS_DISABLE_INT(STATUS_SYS_MODE);

    thread_enqueue(receiver, &readyq);

    sender->t_s.pc = (unsigned int) test_succed_msg_send;
    // SP
    sender->t_s.sp = RAM_TOP - 2*FRAME_SIZE;
    // CPSR -> mask all interrupts and be in kernel mode
    sender->t_s.cpsr = STATUS_DISABLE_INT(STATUS_SYS_MODE);

    thread_enqueue(sender, &readyq);

    thread_count = 2;

    tprint("test_succed_msg_init ended\n");
}


/*********************************************************************************/
