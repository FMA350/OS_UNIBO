#include <interrupts.h>
#include <mikabooq.h>
#include <listx.h>
#include <arch.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <syscall.h>
#include <syslib.h>



/*****EXTERN*****/
extern unsigned int getTIMER();
extern void scheduler();

extern unsigned int *timeSliceLeft;

extern struct list_head readyq;
extern struct tcb_t *current_thread;

static void io_request(unsigned int);
static void interval_timer_h();



void interrupt_h() {
    // TODO: check pending interrupts in priority order
    interval_timer_h();
    //p points to bottom of the interrupt bitmap for external devices
    //il primo byte che ha un interr pendente fa partire la gestione
    void * p = (void *)0x00006fe0;
    unsigned int i = 0;
    while (i < 5){
        if (*((unsigned int *)p)>0) {
            //mi serve sapere quale dei 7 device di ogni tipo ha richiesto l'interr? volendo comunque lo si fa facilmente
            io_request(i);
            break;
        }
        p++;
        i++;
    }

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


static void io_request(unsigned int deviceType){
    //serve?
    #if 0
    switch (deviceType){
        case 0:
            //disk handler
            break;
        case 1:
            //tapes handler
            break;
        case 2:
            //Network handler
            break;
        case 3:
            //printers handler
            break;
        case 4:
            //terminal handler
            break;

    }
    #endif
    // salvataggio stato del processore
    current_thread->t_s = *((state_t *) INT_OLDAREA); //memcpy implicita
    // Inserimento del processo in coda
    thread_enqueue(current_thread, &readyq);
    scheduler();
}
