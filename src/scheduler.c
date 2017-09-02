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
#include <interrupts.h>
#include <syslib.h>
#include <math.h>

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

inline void init_and_load(struct tcb_t *to_load, void *target, unsigned int cpsr) {
    state_init(to_load, target, cpsr);
    // Enqueuing the initialized thread into the ready queue
    thread_enqueue(to_load, &readyq);
    // thread counter is incremented
    thread_count++;
}

/* Loads the ready queue with threads needed by the system */
void load_readyq(struct pcb_t *root) {
    tprint("load_readyq started\n");

    /* Points the thread we want to load */
    struct tcb_t *to_load;
    // SSI deamon thread is loaded into the ready queue
    to_load = ssi_thread_init();
    init_and_load(to_load, ssi, STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE));

    tprintf("SSI thread == %p\n", to_load);

    //triangle_init(root);
    p2test_init(root);

    tprintf("Number of threads in the sistem == %d\n", thread_count);
    tprint("load_readyq finished\n");
}

/* This function is used to handle the case when the ready ready queue is empty
 * inside the scheduler
 * Preconditions: the ready queue is empty
 */
static inline void empty_readyq_h() {
    if (thread_count == 1){
    /* the SSI is the only thread in the system */
    /* shutdown */
        tprint("=== Only SSI in the system ===\n");
        HALT();
    } else if (soft_block_count == 0) {
    /* all process are hard blocked (waiting for msg) */
    /* deadlock */
        tprint("=== deadlock ===\n");
        PANIC();
    } else {
    /* processes in the system are waiting for I/O */
        // tprint("=== processes waiting for IO ===\n");
        setTIMER(clockPerTimeslice); //fma350 test
        setSTATUS(STATUS_ALL_INT_ENABLE(getSTATUS()));
        //setSTATUS(STATUS_ENABLE_INT(getSTATUS()));
        WAIT();
    }
}

void scheduler(){
    current_thread = thread_dequeue(&readyq);
    //tprintf("scheduler started, current is %p\n", current_thread);

    if (current_thread == NULL){
        //tprint("in emptyrq\n");
        empty_readyq_h();

    }

    // BUS_REG_TIME_SCALE = Register that contains the number of clock ticks per microsecond
    // I SECONDI REALI NON CORRISPONDONO AD I SECONDI DEL PROCESSORE EMULATO
    setTIMER(clockPerTimeslice);
    LDST(&current_thread->t_s);
}
