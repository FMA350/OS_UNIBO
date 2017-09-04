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
    timeSliceLeft = getTIMER(); //timeSliceLeft is used by the accountant

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

        tprintf("IT - %p\n", current_thread);
        thread_enqueue(current_thread, &readyq);
    }
    update_clock(clockPerTimeslice);
    scheduler();
}

extern int is_idle;
extern struct tcb_t *soft_blocked_thread[5];
#define TERMINAL_REQUESTER_INDEX    4
#define TRANSM_COMMAND  0xC

/* Non restituisce mai il controllo */
static inline void acknowledge(unsigned int *command_address, int requester_index)
{
    *command_address = DEV_C_ACK;

    // assert(soft_blocked_thread[requester_index]);

    if (soft_blocked_thread[requester_index]) {
        // Sblocchiamo il processo in attesa a nome dell'SSI
        // TODO: il payload del messaggio è lo stato del device?
        int rval = send(soft_blocked_thread[requester_index], SSI, NULL);
        // La send che viene fatta ad un processo in attesa non fallisce mai
        // assert(rval == SEND_SUCCESS);

        soft_blocked_thread[requester_index] = NULL;

        if (is_idle) {
        // se il messaggio è stato inviato mentre il processore era idle
            // assert(sender == SSI);
            is_idle = 0;
            // chiamiamo lo scheduler per schedulare il processo svegliato
            scheduler();
        }
        else
            LDST((state_t *) INT_OLDAREA);
    }
    // else {
    //     tprint("io_h sta cercando di inviare un messaggio a NULL\n");
    //     PANIC();
    // }
}

#define TERMINAL_DEV_FIELD(dev, field) (DEV_REG_ADDR(IL_TERMINAL, dev) + (field))

static inline void io_h(void)
{
    unsigned int *p = (unsigned int *) CDEV_BITMAP_ADDR(IL_TERMINAL);

    // tprint("io_h called\n");

    int i, j;
    for (i = 0; i < 5; i++) {
    //ne controllo uno alla volta di device per gestire un interrupt alla volta
        for (j = 0; j < 8; j++) {
            if (((*p >> j) & 1) == 1) {
            //device n.0 has a pending interrupt
                acknowledge((unsigned int *) TERMINAL_DEV_FIELD(0, TRANSM_COMMAND),
                            TERMINAL_REQUESTER_INDEX);
                tprint("io_h should not arrive here!\n");
                PANIC();
            }
        }
    }
    tprint("End of io_h: io_h should not arrive here!\n");
    PANIC();
}
