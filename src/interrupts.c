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


static inline void interval_timer_h(void);
static inline void io_h(void);

/* Returns 1 if the interrupt int_no is pending */
// #define CAUSE_IP_GET(cause, int_no) ((cause) & (1 << ((int_no) + 24)))

void interrupt_h(void)
{
    //dispatching
    if (CAUSE_IP_GET(((state_t *) INT_OLDAREA)->CP15_Cause, INT_TIMER))
        interval_timer_h();
    else
        io_h();
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
            // se si accettano interrupt dalle tprint questo non va bene
            // assert(soft_blocked_thread[device] != NULL);
            *((unsigned int *) TERMINAL_DEV_FIELD(line, TRANSM_COMMAND)) = DEV_C_ACK;

            if (soft_blocked_thread[device][line]) {
                // Sblocchiamo il processo in attesa a nome dell'SSI
                unsigned int *trs_status = (unsigned int *) TERMINAL_DEV_FIELD(line, TRANSM_STATUS);
                int rval = send(soft_blocked_thread[device][line], SSI, *trs_status);
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
            tprint("io_h - acknowledge: wrong device");
            PANIC();
    }
}

static inline void io_h(void)
{
    int device = IL_TERMINAL;
    unsigned int *p;
    while (!(p = (unsigned int *) CDEV_BITMAP_ADDR(IL_TERMINAL)))
        device--; //scorro nelle bitmap e mi fermo al primo device con interrupt

    int line = 1, a = 1, i;
    for (i = 1; i < 9; i++){ //restituisce il n. di terminale che ha generato l'interrupt
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
