#include <mikabooq.h>
#include <listx.h>
#include <arch.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <libuarm.h>
#include <ssi.h>
#include <debug_tests.h>


/*****EXTERN*****/
extern unsigned int thread_count;
extern unsigned int soft_block_count;

extern struct tcb_t *current_thread;
extern struct list_head readyq;

extern void test_syscall();
extern void BREAKPOINT();

/*****global*****/
unsigned int timeSliceLeft;

unsigned int clockPerTimeslice;


void experimentalClerk(){

    clockPerTimeslice =  (*((unsigned int *) BUS_REG_TIME_SCALE) * (unsigned int) 5000);
    //how many milliseconds did pass?
    if(timeSliceLeft > clockPerTimeslice){
    //5 ms
    }
    else{
        ((timeSliceLeft)*5/clockPerTimeslice);
    }
    tprint("ok\n");
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
    tprint("\nScheduler in action\n\n");
    current_thread = thread_dequeue(&readyq);
    // thread_dequeue sostituito con thread_qhead
    // experimentalClerk();
    //recalculate how many clock cicles 5ms are.
    clockPerTimeslice = (*((unsigned int *) BUS_REG_TIME_SCALE) * (unsigned int) 5000);
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

    // BUS_REG_TIME_SCALE = Register that contains the number of clock ticks per microsecond
    // I SECONDI REALI NON CORRISPONDONO AD I SECONDI DEL PROCESSORE EMULATO
    setTIMER(clockPerTimeslice);
    LDST(&current_thread->t_s);

}
