#include <interrupts.h>
#include <mikabooq.h>
#include <listx.h>
#include <arch.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <syscall.h>
#include <syslib.h>
#include <ssi.h>



/*****EXTERN*****/
extern unsigned int getTIMER();
extern void scheduler();

extern unsigned int *timeSliceLeft;

extern struct list_head readyq;
extern struct tcb_t *current_thread;

static void io_handler();
static void interval_timer_h();



void interrupt_h() {
    // TODO: check pending interrupts in priority order
    interval_timer_h();
    io_handler();

}

/* Interval timer handler without pseudoclock facilities */
static void interval_timer_h() {
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
    //serve?
    //p points to bottom of the interrupt bitmap for external devices
    //il primo byte che ha un interr pendente fa partire la gestione
    void * p = (void *)0x00006ff0;  // non dovrebbe essere 0x00006FE0?
    // dovrebbe funzionare a grandi linee, ma bisognerebbe vedere anche le funzioni
    // del coprocessore (ha un registro che indica la causa degli interrupts)
    int i = 0;
    while (i < 5) {//ne controllo uno alla volta di device per gestire un interrupt alla volta
        if ((*((unsigned int *)p)%2)==1) {//device n.0 has a pending interrupt
            send(SSI,p,i);
            break;
        }else if (((*((unsigned int *)p)>>1)%2)==1){//device n.1 has a pending interrupt
            send(SSI,p,i);
            break;
        }else if (((*((unsigned int *)p)>>2)%2)==1){
            send(SSI,p,i);
            break;
        }else if (((*((unsigned int *)p)>>3)%2)==1){
            send(SSI,p,i);
            break;
        }else if (((*((unsigned int *)p)>>4)%2)==1){
            send(SSI,p,i);
            break;
        }else if (((*((unsigned int *)p)>>5)%2)==1){
            send(SSI,p,i);
            break;
        }else if (((*((unsigned int *)p)>>6)%2)==1){
            send(SSI,p,i);
            break;
        }else if (((*((unsigned int *)p)>>7)%2)==1){
            send(SSI,p,i);
            break;
        }
        i++;
        *((unsigned int *)p) = (unsigned int) p + 2;
        //controlla se funziona!! non sono sicuro
        // non capisco come funziona (michele)

    }
    // salvataggio stato del processore
    //current_thread->t_s = *((state_t *) INT_OLDAREA); //memcpy implicita
    // Inserimento del processo in coda
    //thread_enqueue(current_thread, &readyq);
    scheduler();
}
