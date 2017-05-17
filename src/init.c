#include <mikabooq.h>
#include <listx.h>
#include <scheduler.h>
#include <debug_tests.h>
#include <arch.h>
#include <syslib.h>
#include <interrupts.h>
#include <syscall.h>
#include <trap.h>


void BREAKPOINT(){ }

static void states_init();

static void time_init();

int main() {
    /* Initialization */
    struct pcb_t *root = proc_init();
    thread_init();
    msgq_init();

    /* Loading exceptions states vector */
    states_init();

    time_init();

    /* Loading ready queue with system processes */
    load_readyq(root);
    /* Starting normal life of the system */

    // test();


    scheduler();

    return 0;
}

static void time_init() {
    clockPerTimeslice = (*((unsigned int *) BUS_REG_TIME_SCALE) * (unsigned int) 5000);
}

/* This function is used to populate exception states vector
 * Preconditions: new_area is the address of the new state, while handler
 *                is the function that has to be executed
 */
static inline void LOAD_NEW_STATE(state_t *new_area, void *handler) {
    //FIXME: come vanno caricati gli altri registri?

    STST(new_area);

    new_area->pc = (unsigned int) handler;
    new_area->sp = RAM_TOP;
    new_area->cpsr = STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE);
}

extern void interrupt_h();
extern void tlbtrap_h();
extern void pgmtrap_h();
extern void syscall_h();

static void states_init(){

    LOAD_NEW_STATE((state_t *) INT_NEWAREA, interrupt_h);

    LOAD_NEW_STATE((state_t *) TLB_NEWAREA, tlbtrap_h);

    LOAD_NEW_STATE((state_t *) PGMTRAP_NEWAREA, pgmtrap_h);

    LOAD_NEW_STATE((state_t *) SYSBK_NEWAREA, syscall_h);
}
