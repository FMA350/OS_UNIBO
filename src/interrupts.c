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
static void io_handler();
static void interval_timer_h();



void interrupt_h(){
    //TODO: Enhance control using CPSR (fma350)
    //tprint("$$$ interrupt_h called $$$\n");
    timeSliceLeft = getTIMER();
    //tprintf("timeSliceLeft = %d\n",timeSliceLeft);

    if(current_thread){ //a thread was being executed
        // mnalli: timeSliceLeft è sempre > 0 perché è un unsigned int
        if(((int)timeSliceLeft > 0) && (timeSliceLeft < clockPerTimeslice)){
            io_handler();       //for interrupts
            STATUS_ALL_INT_ENABLE(current_thread->t_s.cpsr); // mnalli: lo statement non ha nessun effetto
            LDST(current_thread);
        }
        else{
            interval_timer_h(); //for fast-interrupts
            STATUS_ALL_INT_ENABLE(current_thread->t_s.cpsr);    // mnalli: lo statement non ha nessun effetto
            thread_enqueue(current_thread, &readyq);
            scheduler();
        }

    }
    else{                //no thread was being executed
        // mnalli: timeSliceLeft è sempre > 0 perché è un unsigned int
        if((timeSliceLeft > 0) && (timeSliceLeft < clockPerTimeslice)){
            io_handler();       //for interrupts
            setSTATUS(STATUS_ALL_INT_ENABLE(STATUS_SYS_MODE));
        }
        else{
            // handle pseudoclock
            update_clock(accountant(NULL));
            setSTATUS(STATUS_ALL_INT_ENABLE(STATUS_SYS_MODE));
            scheduler();
        }
    }
}

static void interval_timer_h(){

    // if(accountant(current_thread) == 5){
    //     //if accountant returns 0, it means the process has consumed all its time
    // //    tprint("current_thread was updated by accountant\n");
    //     //handle pseudoclock
    // }
    // else {
    //     tprintf("$$$ ERROR, THIS shouldn't have happened $$$\n");
    //     //since the condition is checked earlier
    // }

    timeSliceLeft = (unsigned int *) getTIMER();

    if (current_thread) {
    // se deve avvenire il context-switch
        // salvataggio stato del processore
        current_thread->t_s = *((state_t *) INT_OLDAREA); //memcpy implicita
        // Inserimento del processo in coda
        thread_enqueue(current_thread, &readyq);
    }
    scheduler();
}

static inline void io_handler(){
//    tprint("$$$ io_handler called $$$\n");

    //p points to bottom of the interrupt bitmap for external devices
    //il primo byte che ha un interr pendente fa partire la gestione
    // if (current_thread) {
    //     current_thread->t_s = *((state_t *) INT_OLDAREA);
    //     // current_thread->t_s.pc -= 4; //since it skips 4 bytes of instruction
    //     // Inserimento del processo in coda
    //     //thread_enqueue(current_thread, &readyq);
    //     //list_add(&current_thread->t_sched, &readyq);
    // }

    void * p = (void *) 0x00006ff0;

    void * q = p;
    // dovrebbe funzionare a grandi linee, ma bisognerebbe vedere anche le funzioni
    // del coprocessore (ha un registro che indica la causa degli interrupts)
    // void * rcv_cmd = (void *) 0x0000244;
    // *(unsigned int*)rcv_cmd = 1;
    int i = 0;
    while (i < 5) {
    //ne controllo uno alla volta di device per gestire un interrupt alla volta
        if ((*((unsigned int *)p)%2)==1) {
        //device n.0 has a pending interrupt
            //tprintf(">>>>> terminal 0 raised interrupt %p\n",thread_qhead(&blockedq));
//            tprintf("BITMAP: %d\n", *((unsigned int *)p));
            void * trs_cmd = (void *) 0x000024c;
            *(unsigned int*) trs_cmd = 1;
//            tprintf("BITMAP: %d\n", *((unsigned int *)p));
            send(SSI,q,i);
            break;
        } else if (((*((unsigned int *)p)>>1)%2)==1) {
        // device n.1 has a pending interrupt
            send(SSI,p,i);
            break;
        } else if (((*((unsigned int *)p)>>2)%2)==1) {
            send(SSI,p,i);
            break;
        } else if (((*((unsigned int *)p)>>3)%2)==1) {
            send(SSI,p,i);
            break;
        } else if (((*((unsigned int *)p)>>4)%2)==1) {
            send(SSI,p,i);
            break;
        } else if (((*((unsigned int *)p)>>5)%2)==1) {
            send(SSI,p,i);
            break;
        } else if (((*((unsigned int *)p)>>6)%2)==1) {
            send(SSI,p,i);
            break;
        } else if (((*((unsigned int *)p)>>7)%2)==1) {
            send(SSI,p,i);
            break;
        }
        i++;
        *((unsigned int *)p) = (unsigned int) p + 2;
    }
}
