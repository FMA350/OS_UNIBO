#include <interrupts.h>
#include <syslib.h>



/*****EXTERN*****/
extern void syscall_h(int a1, int a2, int a3, int a4); //a1 = syscall a2 =a1
extern unsigned int getTIMER();
extern void scheduler();

extern unsigned int *timeSliceLeft;

extern struct list_head readyq;
extern struct tcb_t *current_thread;


#define NEW_STATE(NEW_AREA) ((state_t *) NEW_AREA)

#define LOAD_NEW_STATE(NEW_AREA, HANDLER_NAME)                                \
    NEW_STATE(NEW_AREA)->pc = (unsigned int) HANDLER_NAME;                    \
    NEW_STATE(NEW_AREA)->cpsr = STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE);      \
    NEW_STATE(NEW_AREA)->sp = RAM_TOP;
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
    // tprint("interrupt_h started\n");
    // check pending interrupts

    interval_timer_h();

}

/* Interval timer handler without pseudoclock facilities */
void interval_timer_h() {
    timeSliceLeft = (unsigned int *)getTIMER();
    // salvataggio stato del processore
    current_thread->t_s = *((state_t *) INT_OLDAREA); //memcpy implicita
    // Inserimento del processo in coda
    thread_enqueue(current_thread, &readyq);
    scheduler();
}
