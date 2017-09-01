#include <interrupts.h>
#include <mikabooq.h>
#include <listx.h>
#include <arch.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <syscall.h>
#include <syslib.h>
#include <ssi.h>
#include <scheduler.h>
#include <libuarm.h>

/*****EXTERN*****/
extern unsigned int getTIMER();
extern void update_clock(unsigned int milliseconds);
extern void scheduler();
extern unsigned int accountant(struct tcb_t* thread);
extern unsigned int clockPerTimeslice;
extern unsigned int timeSliceLeft;

extern struct list_head readyq;
extern struct tcb_t *current_thread;

/*****INTERN******/
int interrupt_flag = 0;
state_t interrupt_t_s; //should it be initialized?
int call_scheduler;

static inline void interval_timer_h();
static inline void io_h();


void interrupt_h(){
    //TODO: Enhance control using CPSR (fma350)
    timeSliceLeft = getTIMER();

    //dispatching
    if((timeSliceLeft > 0) && (timeSliceLeft < clockPerTimeslice)){
        io_h();       //for interrupts
    } else {
        interval_timer_h(); //for fast-interrupts
    }
}

static inline void interval_timer_h(){
    if(current_thread){
        current_thread->t_s = *((state_t *) INT_OLDAREA); //memcpy implicita
        current_thread->run_time += clockPerTimeslice; //cycles
        thread_enqueue(current_thread, &readyq);
    }
    update_clock(clockPerTimeslice);
    scheduler();
}

static inline void io_h(){
//    tprint("$$$ io_h called $$$\n");

    //p points to bottom of the interrupt bitmap for external devices
    //il primo byte che ha un interr pendente fa partire la gestione
    // if (current_thread) {
    //     current_thread->t_s = *((state_t *) INT_OLDAREA);
    //     // current_thread->t_s.pc -= 4; //since it skips 4 bytes of instruction
    //     // Inserimento del processo in coda
    //     //thread_enqueue(current_thread, &readyq);
    //     //list_add(&current_thread->t_sched, &readyq);
    // }

    // tprintf("interval timer interrupt - %d\n", *((unsigned int *) CDEV_BITMAP_ADDR(IL_TIMER)));

    unsigned int *p = (unsigned int *) CDEV_BITMAP_ADDR(IL_TERMINAL);

    // dovrebbe funzionare a grandi linee, ma bisognerebbe vedere anche le funzioni
    // del coprocessore (ha un registro che indica la causa degli interrupts)

    int i;
    for (i = 0; i < 5; i++) {
    //ne controllo uno alla volta di device per gestire un interrupt alla volta
        if ((*p & 1) == 1) {
        //device n.0 has a pending interrupt
            // tprintf(">>>>> terminal 0 raised interrupt %p\n",thread_qhead(&blockedq));
            // tprintf("BITMAP: %d\n", *((unsigned int *)p));
            unsigned int *trs_cmd = (unsigned int *) 0x0000024c;
            *trs_cmd = DEV_C_ACK;
            // tprintf("BITMAP: %d\n", *((unsigned int *)p));
            send(SSI,p,i);
            break;
        } else if (((*p >> 1) & 1) == 1) {
        // device n.1 has a pending interrupt
            send(SSI,p,i);
            break;
        } else if (((*p >> 2) & 1) == 1) {
            send(SSI,p,i);
            break;
        } else if (((*p >> 3) & 1) == 1) {
            send(SSI,p,i);
            break;
        } else if (((*p >> 4) & 1) == 1) {
            send(SSI,p,i);
            break;
        } else if (((*p >> 5) & 1) == 1) {
            send(SSI,p,i);
            break;
        } else if (((*p >> 6) & 1) == 1) {
            send(SSI,p,i);
            break;
        } else if (((*p >> 7) & 1) == 1) {
            send(SSI,p,i);
            break;
        }

        // FIXME: cos'è?
        *p = (unsigned int) p + 2;
    }

    // Si restituisce il controllo al chiamante
    LDST((state_t *) INT_OLDAREA);
}
