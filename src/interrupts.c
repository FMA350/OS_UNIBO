#include <mikabooq.h>
#include <listx.h>
#include <arch.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <interrupts.h>

extern void syscall_h(int a1, int a2, int a3, int a4); //a1 = syscall a2 =a1

#define NEW_STATE(NEW_AREA) ((state_t *) NEW_AREA)

#define LOAD_NEW_STATE(NEW_AREA, HANDLER_NAME)                                       \
    NEW_STATE(NEW_AREA)->pc = (unsigned int) HANDLER_NAME;                    \
    NEW_STATE(NEW_AREA)->cpsr = STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE);     \
    NEW_STATE(NEW_AREA)->sp = RAM_TOP;

void states_init(){
    //TODO: complete loading and care for execution mode
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
    tprint("interrupt_h started\n");
    HALT();
}

void FIQ_Handler(){

}

void IQ_Handler(){

}
