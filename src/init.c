#include <mikabooq.h>
#include <listx.h>
#include <scheduler.h>
#include <interrupts.h>


void BREAKPOINT(){ }

// Thread that has currently the control of the CPU
struct tcb_t *current_thread = NULL;

// sentinella della ready queue
LIST_HEAD(readyq);

// Number of threads currently active in the system
unsigned int thread_count = 0;

/*
 * Soft block: threads that are blocked awaiting for I/O or completion of
 *             a service request by the SSI; they're going to unblock for
 *             themselves in a limited amount of time.
 *
 * Hard block: threads that are blocked awaiting for a message; they need
 *             another process to unblock them.
 */

unsigned int soft_block_count = 0;

int main() {

    /* Initialization */
    struct pcb_t *root = proc_init();
    thread_init();
    msgq_init();

    /* Loading exceptions states vector */
    states_init();

    /* Loading ready queue with system processes */
    load_readyq(root);

    /* Starting normal life of the system */
    scheduler();

    return 0;
}
