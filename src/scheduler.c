#include <mikabooq.h>
#include <listx.h>
#include <arch.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <libuarm.h>
#include <ssi.h>

extern void BREAKPOINT();

extern struct list_head readyq;

extern unsigned int thread_count;

extern unsigned int soft_block_count;

extern struct tcb_t *current_thread;

extern void test_syscall();

void IDLE_proc() {
    // tprint("IDLE_proc started\n");
    WAIT();
}

void test_timer() {
    tprint("test_timer started\n");
    register size_t i, j;
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 100; j++) {
            WAIT();
        }
        BREAKPOINT();
        tprint("100 done\n");
    }
    tprint("terminating test_timer\n");
}

/* TODO: Probabilmente bisogna caricare i demoni necessari al funzionamento del
         sistema nella coda ready in questa funzione. */
void load_readyq(struct pcb_t *root) {
    struct tcb_t *first_thread = thread_alloc(root);
    /* caricare stato di partenza del thread */
    // PC points the thread we are starting
    first_thread->t_s.pc = (unsigned int) test_timer;
    // SP
    first_thread->t_s.sp = RAM_TOP - FRAME_SIZE;
    // CPSR -> mask all interrupts and be in kernel mode
    first_thread->t_s.cpsr = STATUS_DISABLE_INT(STATUS_SYS_MODE);

    // aggiungere il thread alla ready queue a mano
    thread_enqueue(first_thread, &readyq);

    // TODO: inserire SSI thread
}

void scheduler() {
    //tprint("Scheduler in action\n");
    current_thread = thread_dequeue(&readyq);
    // thread_dequeue sostituito con thread_qhead
    // la thread dequeue viene usata nel gestore dell'interval timer (senza pseudoclock)

    if (current_thread == NULL) {
    /* ready queue empty */
        if (thread_count == 1)
        /* the SSI is the only thread in the system */
        /* shutdown */
            HALT();
        else if (soft_block_count == 0)
        /* all process are hard blocked (waiting for msg) */
        /* deadlock */
            PANIC();
        else {
            setSTATUS(STATUS_ALL_INT_ENABLE(STATUS_SYS_MODE));
            WAIT();
        }
    }
    // tprint(" Got next thread to execute\n"
    //        " Jumping...\n");

    // BUS_REG_TIME_SCALE = Register that contains the number of clock ticks per microsecond
    // I SECONDI REALI NON CORRISPONDONO AD I SECONDI DEL PROCESSORE EMULATO
    setTIMER(* ((unsigned int *) BUS_REG_TIME_SCALE) * ((unsigned int) 5000));
    BREAKPOINT();
    LDST(&current_thread->t_s);

}
