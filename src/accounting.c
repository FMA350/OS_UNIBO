#include <mikabooq.h>
#include <nucleus.h>
#include <arch.h>
#include <scheduler.h>
#include <handlers.h>
#include <math.h>

#include "syslib.h"
#include "accounting.h"

STOPWATCH(sys_stopwatch);
unsigned int timeSliceLeft;
unsigned int cyclesUsed;
unsigned int TICKS_PER_TIME_SLICE;
// sentinella della coda dei processi richiedenti il servizio di attesa
LIST_HEAD(t_wait4clock);
unsigned int pseudoclock = 0;


//returns the time passed in microseconds.
unsigned int accountant(struct tcb_t* thread)
{
    uint64_t cycles = thread->run_time;
    int milliseconds = 0;
    cycles -= 500; // for rounding purpouses
    cycles /= 1;
        //how many 1000 to remove before it goes in underflow?
        while(cycles > 0) {
            milliseconds++;
            cycles -= 1000;
        }
    return milliseconds;
}


#define PSEUDOCLOCK_TICK 100000    // ogni 100 ms lo pseudoclock fa un tick

void update_clock(unsigned int cycles)
{
    if((pseudoclock += cycles) >= PSEUDOCLOCK_TICK) {
        pseudoclock -= PSEUDOCLOCK_TICK;

        struct tcb_t *to_resume;
        while((to_resume = thread_qhead(&t_wait4clock)) != NULL) {
            if (to_resume->t_status == T_STATUS_W4MSG) {
                int rval = send(to_resume, SSI, (uintptr_t) NULL);
                // La send che viene fatta ad un processo in attesa non fallisce mai
                assert(rval == SEND_SUCCESS);
            } else {

            // to_resume->t_status == T_STATUS_READY
                // rimosso dalla lista t_wait4clock
                thread_outqueue(to_resume);
                // inserito nella lista di processi da schedulare
                thread_enqueue(to_resume, &readyq);
                int rval = send(to_resume, SSI, (uintptr_t) NULL);
                // La send fallisce nel caso in cui finiscono i messaggi
                assert(rval == SEND_SUCCESS);
            }
        soft_block_count--;
        }
    }
}
