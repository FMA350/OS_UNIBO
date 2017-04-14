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


#define NEW_STATE(NEW_AREA) ((state_t *) NEW_AREA)

#define LOAD_NEW_STATE(NEW_AREA, HANDLER_NAME)                                   \
    ((state_t *) NEW_AREA)->pc = (unsigned int) HANDLER_NAME;                    \
    ((state_t *) NEW_AREA)->cpsr = STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE);      \
    ((state_t *) NEW_AREA)->sp = RAM_TOP;                                        \
    // NEW_STATE(NEW_AREA)->sl = ???;

void states_init(){
    //TODO: complete loading and care for execution mode; stack limit register???
    //FIXME: come vanno caricati gli altri registri?

    LOAD_NEW_STATE(INT_NEWAREA, interrupt_h);

    // TLB_NEWAREA
    // TODO: Settare HANDLER_NAME
    // LOAD_NEW_STATE(TLB_NEWAREA, HANDLER_NAME)

    // PGMTRAP_NEWAREA
    // TODO: Settare HANDLER_NAME
    // LOAD_NEW_STATE(PGMTRAP_NEWAREA, HANDLER_NAME)

    LOAD_NEW_STATE(SYSBK_NEWAREA, syscall_h);
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
