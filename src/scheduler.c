#include <mikabooq.h>
#include <listx.h>
#include <arch.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <libuarm.h>
#include <ssi.h>
#include <debug_tests.h>
#include <nucleus.h>
#include <scheduler.h>
#include <p2test.h>



// Thread that has currently the control of the CPU
struct tcb_t *current_thread = NULL;

// sentinella della ready queue
LIST_HEAD(readyq);

// Number of threads currently active in the system
unsigned int thread_count = 0;

/*
 * Soft block: threads that are blocked awaiting for I/O or completion of
 *             a service request by the SSI; they're going to unblock for
 *             themselves in a limited amount of time.
 *
 * Hard block: threads that are blocked awaiting for a message; they need
 *             another process to unblock them.
 */

unsigned int soft_block_count = 0;


extern void test_syscall();
extern void BREAKPOINT();

/*****global*****/
unsigned int timeSliceLeft;

// the value in BUS_REG_TIME_SCALE is the same for the whole execution
unsigned int clockPerTimeslice;

#if 0
void accountant(struct tcb_t* thread){
/*                               _              _
                                | |            | |
  __ _  ___ ___ ___  _   _ _ __ | |_ __ _ _ __ | |_
 / _` |/ __/ __/ _ \| | | | '_ \| __/ _` | '_ \| __|
| (_| | (_| (_| (_) | |_| | | | | || (_| | | | | |_
\__,_|\___\___\___/ \__,_|_| |_|\__\__,_|_| |_|\__|

*/
    //how many milliseconds did pass?
    if(timeSliceLeft >= clockPerTimeslice){
        //5 ms in total
        thread->runTime += 5;
    }
    else{
        thread->runTime += (5-((timeSliceLeft)*5/clockPerTimeslice));
    }
}
#endif

/* Loads the initial thread state
 *
 * Preconditions: to_load is the thread that has to be initialized, target is
 * the function that the thread has to execute
 */
 //ssi --> STATUS_ALL_INT_DISABLE
 //other kernel thread --> other status
static inline void state_init(struct tcb_t *to_load, void *target, unsigned int cpsr) {
    static unsigned int n = 1;

    STST(&to_load->t_s);

    // PC points the thread we are starting
    to_load->t_s.pc = (unsigned int) target;
    // SP
    to_load->t_s.sp = RAM_TOP - n*FRAME_SIZE;

    to_load->t_s.cpsr = cpsr;

    n++;
}

/* Loads the initial thread state and loads the thread in the ready queue  */
inline void init_and_load(struct tcb_t *to_load, void *target, unsigned int status) {
    state_init(to_load, target, status);
    // aggiungere il thread alla ready queue a mano
    thread_enqueue(to_load, &readyq);
    thread_count++;
}

/* Loads the ready queue with the threads needed by the system */
void load_readyq(struct pcb_t *root) {

    /* Points to the thread we want to load*/
    struct tcb_t *to_load;
    to_load = ssi_thread_init();
    init_and_load(to_load, ssi, STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE));
    //triangle_init(root);
    p2test_init(root);
}

/* This function is used to handle the case when the ready ready queue is empty
 * inside the scheduler
 * Preconditions: the ready queue is empty
 */
static inline void empty_readyq() {
    if (thread_count == 1)
    /* the SSI is the only thread in the system */
    /* shutdown */
        HALT();
#if 0
    else if (soft_block_count == 0){ //per il momento lo tolgo
    /* all process are hard blocked (waiting for msg) */
    /* deadlock */
        tprint("deadlock\n");
        PANIC();
    }
#endif
    else {
    /* processes in the system are waiting for I/O */
        tprintf("\tprocesses waiting for IO\n");
        setSTATUS(STATUS_ALL_INT_ENABLE(STATUS_SYS_MODE));
        WAIT();
    }
}

void scheduler() {
    //accountant(current_thread);
    current_thread = thread_dequeue(&readyq);
    // accountant();
    tprintf("\n\tscheduler started, current is %p\n", current_thread);

    if (current_thread == NULL)
        empty_readyq();

    // BUS_REG_TIME_SCALE = Register that contains the number of clock ticks per microsecond
    // I SECONDI REALI NON CORRISPONDONO AD I SECONDI DEL PROCESSORE EMULATO
    setTIMER(clockPerTimeslice);
    LDST(&current_thread->t_s);
}
