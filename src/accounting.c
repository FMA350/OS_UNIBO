#include <mikabooq.h>
#include <nucleus.h>
#include "accounting.h"

#define PSEUDOCLOCK_TICK 100

/*****global*****/
unsigned int timeSliceLeft;
// the value in BUS_REG_TIME_SCALE is the same for the whole execution
unsigned int clockPerTimeslice = 5000; //at 1mHz for 5 milliseconds

// sentinella della coda dei processi richiedenti il servizio di attesa
LIST_HEAD(t_wait4clock);

unsigned int pseudoclock = 0;


unsigned int accountant(struct tcb_t* thread){
    //returns the time passed in milliseconds.
    int cycles = thread->run_time;
    int milliseconds = 0;
        cycles -= 500; // for rounding purpouses
        //how many 1000 to remove before it goes in underflow?
        while(cycles > 0){
            milliseconds++;
            cycles-=1000;
        }
    return milliseconds;
}

void update_clock(unsigned int milliseconds){
    pseudoclock += milliseconds;
    if(pseudoclock >= PSEUDOCLOCK_TICK){
        pseudoclock -= PSEUDOCLOCK_TICK;
        struct tcb_t *to_resume;
        while((to_resume = thread_dequeue(&t_wait4clock))!=NULL){
            msgsend(to_resume,NULL);
        }
    }
}
