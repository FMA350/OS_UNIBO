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

    load_readyq(root);
    tprint(" First thread allocated\n");

    scheduler();

    return 0;
}
