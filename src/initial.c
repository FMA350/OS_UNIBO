#include <mikabooq.h>
#include <listx.h>
#include <arch.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <scheduler.h>
#include <interrupts.h>


void BREAKPOINT(){ }

extern void test_wait();

extern void test_syscall();

void syscall_h(int a1, int a2, int a3, int a4){
    // BREAKPOINT();
    if (a1 == 1)
        tprint("a1 == 1\n");
    else
        tprint("a1 != 1\n");

    if (a2 == 1)
        tprint("a2 == 1\n");
    else
        tprint("a2 != 1\n");

    if (a3 == 1)
        tprint("a3 == 1\n");
    else
        tprint("a3 != 1\n");

    if (a4 == 1)
        tprint("a4 == 1\n");
    else
        tprint("a4 != 1\n");

    tprint("====================================\n");
    LDST((void *) SYSBK_OLDAREA);
}

// the thread that has currently the control of the CPU
struct tcb_t *current_thread = NULL;

// sentinella della ready queue
LIST_HEAD(readyq);

// the number of threads in the system
unsigned int thread_count = 0;

// the number of threads that are blocked awaiting for I/O or
// completion of a service request by the SSI
unsigned int soft_block_count = 0;

int main(int argc, char const *argv[]) {
    tprint(" main starting...\n");

    /* Initialization */
    struct pcb_t *root = proc_init();
    thread_init();
    msgq_init();

    tprint(" Data structures initialized\n");

    states_init();

    load_readyq(thread_alloc(root));
    tprint(" First thread allocated\n");

    scheduler();

    return 0;
}
