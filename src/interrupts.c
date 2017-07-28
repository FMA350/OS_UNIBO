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

    tprint("$$$ interrupt_h started $$$\n");

    // TODO: check pending interrupts in priority order
    io_handler();
    interval_timer_h();

    tprint("$$$ interrupt_h finished $$$\n");
}

/*fma: Interval timer handler without pseudoclock */
static void interval_timer_h(){
    timeSliceLeft = (unsigned int *) getTIMER();

    if (current_thread) {
    // se deve avvenire il context-switch
        // salvataggio stato del processore
        current_thread->t_s = *((state_t *) INT_OLDAREA);
        accountant(current_thread);
        // Inserimento del processo in coda
        thread_enqueue(current_thread, &readyq);
    }
    //fma FIXME: it should jump to it. does it do that? (like a goto)
    scheduler();
}


static inline void io_handler(){
    // Manual section 3.1.6
    //p points to bottom of the interrupt bitmap for external devices

    void * p = (void *) 0x00006fe0; //fma: Corretto errore, l'indirizzo da cui partire era
                                    //settato a 0x00006ff0 (indirizzo del terminale)
    // dovrebbe funzionare a grandi linee, ma bisognerebbe vedere anche le funzioni
    // del coprocessore (ha un registro che indica la causa degli interrupts)
    int i = 0;
    while (i < 5) {
    //ne controllo uno alla volta di device per gestire un interrupt alla volta
    //TODO: SSI is not acknowledged on which of the 8 devices of a class is throwing it.
        if ((*((unsigned int *)p)%2)==1) {
        //device n.0 has a pending interrupt
            tprint(">>> terminal 0 raised interrupt\n");
            send(SSI,p,i);
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
        //controlla se funziona!! non sono sicuro
        // non capisco come funziona (michele)

    }
    // salvataggio stato del processore
    //current_thread->t_s = *((state_t *) INT_OLDAREA); //memcpy implicita
    // Inserimento del processo in coda
    //thread_enqueue(current_thread, &readyq);
    //scheduler();
}
