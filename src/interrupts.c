#include <mikabooq.h>
#include <listx.h>
#include <arch.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <syslib.h>
#include <ssi.h>
#include <scheduler.h>
#include <libuarm.h>
#include <nucleus.h>
#include <do_io.h>
#include "handlers.h"
#include "accounting.h"


/*****INTERN******/
// int interrupt_flag = 0;
// state_t interrupt_t_s; //should it be initialized?
// int call_scheduler;


static inline void interval_timer_h(void);
static inline void io_h(void);

void interrupt_h(void)
{
    //TODO: Enhance control using CPSR (fma350)
    timeSliceLeft = getTIMER();
    //dispatching
    if((timeSliceLeft > 0) && (timeSliceLeft < TICKS_PER_TIME_SLICE)){
        io_h();             //for interrupts
    } else {
        interval_timer_h(); //for fast-interrupts
    }
}

static inline void interval_timer_h(void)
{
    if(current_thread) {
        current_thread->t_s = *((state_t *) INT_OLDAREA);
        current_thread->run_time += TICKS_PER_TIME_SLICE; //cycles

        thread_enqueue(current_thread, &readyq);
    }
    update_clock(TICKS_PER_TIME_SLICE);
    scheduler();
}

/* Non restituisce mai il controllo
 * risveglia i
 *
 *
 *
 */
static inline void acknowledge(unsigned int line, unsigned int device)
{
    switch (device) {
        case EXT_IL_INDEX(IL_TERMINAL):
            assert(soft_blocked_thread[device] != NULL);
            unsigned int *command_address = (unsigned int *) TERMINAL_DEV_FIELD(line, TRANSM_COMMAND);
            *command_address = DEV_C_ACK;
            //tprintf("linea = %d",line);
            if (soft_blocked_thread[device]) {
                // Sblocchiamo il processo in attesa a nome dell'SSI
                void * trs_status = TERMINAL_DEV_FIELD(line, TRANSM_STATUS);
                int rval = send(soft_blocked_thread[device][line], SSI, *(unsigned int*)trs_status);
                soft_block_count--;

                // La send che viene fatta ad un processo in attesa non fallisce mai
                assert(rval == SEND_SUCCESS);

                soft_blocked_thread[device][line] = NULL;

                if (is_idle){
                // se il messaggio è stato inviato mentre il processore era idle
                    is_idle = 0;
                    // chiamiamo lo scheduler per schedulare il processo svegliato
                    scheduler();
                } else
                    LDST((state_t *) INT_OLDAREA);
            } else
            // se l'interrupt proviene da una tprint
                // siccome non abbiamo svegliato nessuno, dobbiamo per forza caricare il vecchio stato
                LDST((state_t *) INT_OLDAREA);
            break;

        default:
            PANIC();
    }
}

static inline void io_h(void)
{
    int device = IL_TERMINAL;
    unsigned int *p;
    while (!(p = (unsigned int *) CDEV_BITMAP_ADDR(IL_TERMINAL)))
        device--; //scorro nelle bitmap e mi fermo al primo device con interrupt

    int line = 1;
    int a = 1;
    for (int i = 1; i < 9; i++){ //restituisce il n. di terminale che ha generato l'interrupt
        a = a*2;
        if (a > *p){
            line=i-1;
            break;
        }
    }
    acknowledge(line, EXT_IL_INDEX(device));

    tprint("io_h should not arrive here!\n");
    PANIC();
}
