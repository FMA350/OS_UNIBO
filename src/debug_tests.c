#include <libuarm.h>
#include <sys/types.h>
#include <nucleus.h>
#include <arch.h>
#include <debug_tests.h>
#include <scheduler.h>
#include <p2test.h>

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

#include <syslib.h>

void test_fail_msg() {
    tprint("test_fail_msg started\n");

    int rval = msgsend(NULL, 1);

    BREAKPOINT();

    tprintf("msgsend should fail (return -1) --> rval == %d\n", rval);

    HALT();
}

/******************************************************************************/

struct tcb_t *sender;
struct tcb_t *receiver;

extern struct list_head readyq;

void test_succed_msg_recv() {
    tprint("test_succed_msg_recv started\n");
    uintptr_t store;
    struct tcb_t *sender_of_msg = msgrecv(NULL, &store);

    tprintf("The message value received should be 10 --> value == %d\n", (int) store);

    tprint("test_succed_msg_recv terminated\n");
    // HALT();
    msgrecv(NULL, &store);
}


void test_succed_msg_send() {
    tprint("test_succed_msg_send started\n");
    int rval = msgsend(receiver, 10);
    tprintf("msgsend should succeed (return 0) --> rval == %d\n", rval);

    // stop the process
    uintptr_t store;
    msgrecv(NULL, NULL);
}


void test_succed_msg_init(struct pcb_t *root) {
    tprint("test_succed_msg_init started\n");

    sender = thread_alloc(root);
    receiver = thread_alloc(root);

    init_and_load(sender, test_succed_msg_send, STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE));
    init_and_load(receiver, test_succed_msg_recv, STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE));

    tprint("test_succed_msg_init ended\n\n");
}


/*********************************************************************************/

struct tcb_t *first, *second, *third;

void t1() {
    msgsend(second, 1);
    int msg;
    if (msgrecv(third, &msg) == NULL)
        tprint("thread 1: error receiving from thread 3\n");
    else
        tprintf("thread 1 (%p) received from thread 3 (%p) msg --> %d\n", first, third, msg);

    terminate_thread();
}

void t2() {
    msgsend(third, 2);
    int msg;
    if(msgrecv(first, &msg) == NULL)
        tprint("thread 2: error receiving from thread 1\n");
    else
        tprintf("thread 2 (%p) received from thread 1 (%p) msg --> %d\n", second, first, msg);
    terminate_thread();
}

void t3() {
    msgsend(first, 3);
    int msg;
    if(msgrecv(second, &msg) == NULL)
        tprint("thread 3: error receiving from thread 2\n");
    else
        tprintf("thread 3 (%p) received from thread 2 (%p) msg --> %d\n", third, second, msg);
    terminate_thread();
}

void triangle_init(struct pcb_t *root) {
    tprint("triangle_init started\n");

    first = thread_alloc(root);
    second = thread_alloc(root);
    third = thread_alloc(root);


    init_and_load(first, t1, STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE));
    init_and_load(second, t2, STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE));
    init_and_load(third, t3, STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE));


    tprint("triangle_init ended\n\n");
}

void p2test_init(struct pcb_t *root){
    // tprint("p2test_init started\n");

    // root process is filled with a working thread
    struct tcb_t *thread_test = thread_alloc(root);
    // test thread is initialized and loaded
    init_and_load(thread_test, test, STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE));

    // tprintf("test thread == %p\n", thread_test);

    // tprint("p2test_init finished\n");
}
