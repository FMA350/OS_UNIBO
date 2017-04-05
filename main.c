#include <mikabooq.h>
#include <listx.h>
#include <arch.h>
#include <uARMconst.h>
#include <uARMtypes.h>



extern void test();

void interrupt_h(){
    tprint("interrupt_h has started!");
    HALT();
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
/*
    // Reset -- 0x00000000
    // Undefined Instruction
    *((unsigned int *) 0x00000004) = 0;
    // Software Interrupt
    *((unsigned int *) 0x00000008) = 0;
    // Prefetch Abort
    *((unsigned int *) 0x0000000C) = 0;
    // Data Abort
    *((unsigned int *) 0x00000010) = 0;
    // reserved/unused -- 0x00000014
    // Interrupt Request
    *((unsigned int *) 0x00000018) = 0;
    // Fast Interrupt Request
    *((unsigned int *) 0x0000001C) = 0;
*/

    /* Loading exception states vector */
    ((state_t *) INT_NEWAREA)->pc = (unsigned int) interrupt_h;
    setTIMER(1);

    WAIT();

    struct tcb_t *first_thread = thread_alloc(root);
    tprint(" First thread allocated\n");

    /* caricare stato di partenza del thread */
    // PC points the thread we are starting
    first_thread->t_s.pc = (unsigned int) test;
    // SP
    first_thread->t_s.sp = RAM_TOP - FRAME_SIZE;
    // CPSR -> mask all interrupts and be in kernel mode
    first_thread->t_s.cpsr = STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE);

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
