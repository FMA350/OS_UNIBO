#include <stopwatch.h>

#include <arch.h>
#include <scheduler.h>
#include <handlers.h>
#include <syslib.h>

static uint64_t next_tick_time;
LIST_HEAD(t_wait4clock);

// Number of clock cycles per pseudoclock tick
static unsigned int TICKS_PER_PSEUDOCLOCK_TICK;

void pseudoclock_init(void) {
    TICKS_PER_PSEUDOCLOCK_TICK = *((unsigned int *) (BUS_REG_TIME_SCALE)) * 100000;
    next_tick_time = 0 + TICKS_PER_PSEUDOCLOCK_TICK;
}

static inline void update_next_tick_time(void) {
    next_tick_time += TICKS_PER_PSEUDOCLOCK_TICK;
}

static inline void do_tick(void)
{
    update_next_tick_time();

    struct tcb_t *to_resume;

    while((to_resume = thread_qhead(&t_wait4clock)) != NULL) {
        if (to_resume->t_status == T_STATUS_W4MSG) {
            int rval = send(to_resume, SSI, (uintptr_t) NULL);
            // La send che viene fatta ad un processo in attesa non fallisce mai
            assert(rval == SEND_SUCCESS);
        } else {
        // to_resume->t_status == T_STATUS_READY
            // rimosso dalla lista t_wait4clock
            move_thread(to_resume, &readyq);
            int rval = send(to_resume, SSI, (uintptr_t) NULL);
            // La send fallisce nel caso in cui finiscono i messaggi
            assert(rval == SEND_SUCCESS);
        }
    soft_block_count--;
    }
}

void pseudoclock_check(void)
{
    //FIXME: problema ordine registri
    uint64_t now = pack((uint32_t) getTODHI(), (uint32_t) getTODLO());
    if (now >= next_tick_time)
        do_tick();
}

uint32_t pseudoclock_time_to_next_tick(void) {
    uint64_t now = pack((uint32_t) getTODHI(), (uint32_t) getTODLO());
    if (now >= next_tick_time)
        return ((uint32_t) 0);
    else
        return ((uint32_t) (next_tick_time - now));
}
