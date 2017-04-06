#include <mikabooq.h>
#include <listx.h>
#include <arch.h>
#include <uARMconst.h>
#include <uARMtypes.h>


void breakpoint(){ }

extern void test_wait();

extern void test_syscall();

void interrupt_h(){
    tprint("interrupt_h has started!\n");
    HALT();
}

void syscall_h(a1, a2, a3, a4){
    breakpoint();
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



int main(int argc, char const *argv[]) {
    tprint(" main starting...\n");
    // sentinella della ready queue
    LIST_HEAD(readyq);

    /* Initialization */
    struct pcb_t *root = proc_init();
    thread_init();
    msgq_init();

    tprint(" Data structures initialized\n");

    /* TODO: load exception vector correctly
    The first bus addresses (0x0000.0000 → 0x0000.001C) are occupied by
    the fast exception vectors.*/


    /* Loading exception states vector */

    ((state_t *) INT_NEWAREA)->pc = (unsigned int) interrupt_h;
    ((state_t *) INT_NEWAREA)->cpsr = STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE);
    ((state_t *) INT_NEWAREA)->sp = RAM_TOP - FRAME_SIZE;
    //setTIMER(0x000FFFFF);

    ((state_t *) SYSBK_NEWAREA)->pc = (unsigned int) syscall_h;
    ((state_t *) SYSBK_NEWAREA)->cpsr = STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE);
    ((state_t *) SYSBK_NEWAREA)->sp = RAM_TOP - FRAME_SIZE;

    struct tcb_t *first_thread = thread_alloc(root);
    tprint(" First thread allocated\n");

    /* caricare stato di partenza del thread */
    // PC points the thread we are starting
    first_thread->t_s.pc = (unsigned int) test_syscall;
    // SP
    first_thread->t_s.sp = RAM_TOP - FRAME_SIZE;
    // CPSR -> mask all interrupts and be in kernel mode
    first_thread->t_s.cpsr = STATUS_DISABLE_INT(STATUS_SYS_MODE);

    tprint(" Thread status set\n");

    // aggiungere il thread alla ready queue a mano
    thread_enqueue(first_thread, &readyq);
    tprint(" Thread enqueued\n");

    /* chiamare lo scheduler */
    // TODO: scheduler
    struct tcb_t *runner = thread_dequeue(&readyq);
    if (runner == NULL) {
        tprint("Empty list\n");
        PANIC();
    }

    tprint(" Got next thread to execute\n");
    tprint(" Jumping...\n");
    LDST(&runner->t_s);

    return 0;
}
