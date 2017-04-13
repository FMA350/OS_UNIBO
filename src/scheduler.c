#include <mikabooq.h>
#include <listx.h>
#include <arch.h>
#include <uARMconst.h>
#include <uARMtypes.h>

extern void BREAKPOINT();

extern struct list_head readyq;

extern unsigned int thread_count;

extern unsigned int soft_block_count;

extern void test_syscall();

void IDLE_proc() {
    tprint("IDLE_proc started\n");
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
         sistema nella coda ready in questa funzione. Un nome migliore potrebbe
         essere load_readyq */
void first_thread_launch(struct tcb_t *first_thread) {

    /* caricare stato di partenza del thread */
    // PC points the thread we are starting
    first_thread->t_s.pc = (unsigned int) test_timer;
    // SP
    first_thread->t_s.sp = RAM_TOP - FRAME_SIZE;
    // CPSR -> mask all interrupts and be in kernel mode
    first_thread->t_s.cpsr = STATUS_DISABLE_INT(STATUS_SYS_MODE);

    // tprint(" Thread status set\n");

    // aggiungere il thread alla ready queue a mano
    thread_enqueue(first_thread, &readyq);
    // tprint(" Thread enqueued\n");
}

void scheduler() {
    //tprint("Scheduler in action\n");
    struct tcb_t *runner = thread_qhead(&readyq);
    // thread_dequeue sostituito con thread_qhead
    // la thread dequeue viene usata nel gestore dell'interval timer (senza pseudoclock)

    if (runner == NULL) {
    /* ready queue empty */
        if (thread_count == 1)
        /* the SSI is the only thread in the system */
        /* shutdown */
            HALT();
        else if (soft_block_count == 0)
        /* deadlock */
            PANIC();
        else
        //FIXME: scheduler runs with all interrupts disabled; WAIT couldn't work
            WAIT();
    }
    // tprint(" Got next thread to execute\n"
    //        " Jumping...\n");

    // BUS_REG_TIME_SCALE = Register that contains the number of clock ticks per microsecond
    // I SECONDI REALI NON CORRISPONDONO AD I SECONDI DEL PROCESSORE EMULATO
    setTIMER(* ((unsigned int *) BUS_REG_TIME_SCALE) * ((unsigned int) 1000));
    BREAKPOINT();
    LDST(&runner->t_s);
}
