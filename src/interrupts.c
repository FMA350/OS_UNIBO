#include <interrupts.h>
#include <mikabooq.h>
#include <listx.h>
#include <arch.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <syscall.h>
#include <syslib.h>



/*****EXTERN*****/
extern unsigned int getTIMER();
extern void scheduler();

extern unsigned int *timeSliceLeft;

extern struct list_head readyq;
extern struct tcb_t *current_thread;

static void interval_timer_h();


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

void states_init(){

    LOAD_NEW_STATE((state_t *) INT_NEWAREA, interrupt_h);

    // TODO: Settare HANDLER_NAME
    // LOAD_NEW_STATE((state_t *) TLB_NEWAREA, HANDLER_NAME)

    // TODO: Settare HANDLER_NAME
    // LOAD_NEW_STATE((state_t *) PGMTRAP_NEWAREA, HANDLER_NAME)

    LOAD_NEW_STATE((state_t *) SYSBK_NEWAREA, syscall_h);
}

void interrupt_h() {
    // TODO: check pending interrupts in priority order
    interval_timer_h();
}

/* Interval timer handler without pseudoclock facilities */
static void interval_timer_h() {
    timeSliceLeft = (unsigned int *) getTIMER();
    // salvataggio stato del processore
    current_thread->t_s = *((state_t *) INT_OLDAREA); //memcpy implicita
    // Inserimento del processo in coda
    thread_enqueue(current_thread, &readyq);
    scheduler();
}
