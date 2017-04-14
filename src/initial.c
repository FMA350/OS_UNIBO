#include <mikabooq.h>
#include <listx.h>
#include <arch.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <scheduler.h>
#include <interrupts.h>


void BREAKPOINT(){ }

// the thread that has currently the control of the CPU
struct tcb_t *current_thread = NULL;

// sentinella della ready queue
LIST_HEAD(readyq);

// the number of threads in the system
unsigned int thread_count = 0;

/*
 * Soft block: threads that are blocked awaiting for I/O or completion of
 * a service request by the SSI
 *
 * Hard block: threads that are blocked awaiting for a message
 */
unsigned int soft_block_count = 0;

int main() {

    /* Initializing data structures */
    struct pcb_t *root = proc_init();
    thread_init();
    msgq_init();

    /* Loading exceptions state vector */
    states_init();

    /* Loading ready queue with system processes */
    load_readyq(root);

    /* Starting normal life of the system */
    scheduler();

    return 0;
}
