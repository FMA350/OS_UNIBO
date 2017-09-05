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
        io_h();       //for interrupts
    } else {
        interval_timer_h(); //for fast-interrupts
    }
}

static inline void interval_timer_h(void)
{
    if(current_thread) {
        current_thread->t_s = *((state_t *) INT_OLDAREA);
        current_thread->run_time += TICKS_PER_TIME_SLICE; //cycles

        // tprintf("IT - %p\n", current_thread);
        thread_enqueue(current_thread, &readyq);
    }
    update_clock(TICKS_PER_TIME_SLICE);
    scheduler();
}

/* Non restituisce mai il controllo */
static inline void acknowledge(unsigned int *command_address, int requester_index)
{
    *command_address = DEV_C_ACK;

    // assert(soft_blocked_thread[requester_index] != NULL);

    if (soft_blocked_thread[requester_index]) {
        // Sblocchiamo il processo in attesa a nome dell'SSI
        void * trs_status = (void *) 0x0000248;
        int rval = send(soft_blocked_thread[requester_index], SSI, *(unsigned int*)trs_status);
        soft_block_count--;

        // La send che viene fatta ad un processo in attesa non fallisce mai
        assert(rval == SEND_SUCCESS);

        soft_blocked_thread[requester_index] = NULL;

        if (is_idle) {
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
}

static inline void io_h(void)
{
    unsigned int *p = (unsigned int *) CDEV_BITMAP_ADDR(IL_TERMINAL);

    assert((*p & 1) == 1);
    //device n.0 has a pending interrupt
    acknowledge((unsigned int *) TERMINAL_DEV_FIELD(0, TRANSM_COMMAND),
                TERMINAL_REQUESTER_INDEX);

    tprint("io_h should not arrive here!\n");
    PANIC();
}
