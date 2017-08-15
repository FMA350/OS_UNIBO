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



/*****EXTERN*****/
extern unsigned int getTIMER();
extern void scheduler();

extern unsigned int *timeSliceLeft;

extern struct list_head readyq;
extern struct tcb_t *current_thread;

static void io_handler();
static void interval_timer_h();



void interrupt_h() {
    //tprint("$$$ interrupt_h started $$$\n");

    // TODO: check pending interrupts in priority order
    io_handler();
    interval_timer_h();

    //tprint("$$$ interrupt_h finished $$$\n");
}

/* Interval timer handler without pseudoclock facilities */
static void interval_timer_h() {
    timeSliceLeft = (unsigned int *) getTIMER();

    if (current_thread) {
    // se deve avvenire il context-switch
        // salvataggio stato del processore
        current_thread->t_s = *((state_t *) INT_OLDAREA);
        // Inserimento del processo in coda
        thread_enqueue(current_thread, &readyq);
    }
    scheduler();
}

static inline void io_handler(){
    //tprintf("INTERRUPT HANDLER\n");
    //serve?
    //p points to bottom of the interrupt bitmap for external devices
    //il primo byte che ha un interr pendente fa partire la gestione
    if (current_thread) {
        // salvataggio stato del processore
        //tprintf("%p state saved\n", current_thread);
        current_thread->t_s = *((state_t *) INT_OLDAREA); //memcpy implicita
        // Inserimento del processo in coda
        //thread_enqueue(current_thread, &readyq);
        list_add(&current_thread->t_sched, &readyq);
    }
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


    //scheduler();
}
