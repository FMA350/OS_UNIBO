#include <mikabooq.h>
#include <listx.h>
#include <scheduler.h>
#include <interrupts.h>
#include <debug_tests.h>


void BREAKPOINT(){ }

int main() {

    /* Initialization */
    struct pcb_t *root = proc_init();
    thread_init();
    msgq_init();

    /* Loading exceptions states vector */
    states_init();

    /* Loading ready queue with system processes */
    // load_readyq(root);
    test_succed_msg_init(root);

    /* Starting normal life of the system */
    scheduler();

    return 0;
}
