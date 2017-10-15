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
#include "time.h"

#include <pseudoclock.h>


static inline void interval_timer_h(void);
static inline void io_h(void);

// dispatch
void interrupt_h(void)
{
    if (CAUSE_IP_GET(((state_t *) INT_OLDAREA)->CP15_Cause, INT_TIMER))
        interval_timer_h();
    else
        io_h();
}

static inline void interval_timer_h(void)
{
    if (current_thread) {
        current_thread->t_s = *((state_t *) INT_OLDAREA);
        current_thread->run_time += (uint64_t) stopwatch_stop(&sys_stopwatch);
        stopwatch_reset(&sys_stopwatch);
        move_thread(current_thread, &readyq);
    }

    pseudoclock_check();
    scheduler();
}


static inline void acknowledge(unsigned int line, unsigned int device);

// Precondition: there is a device interrupt pending
static inline void io_h(void)
{
    // The lower interrupt lines have the higher priority
    unsigned int current_interrupt_line = DEV_IL_START;

    while (!CAUSE_IP_GET(((state_t *) INT_OLDAREA)->CP15_Cause, current_interrupt_line))
        current_interrupt_line++;

    unsigned int p = *((unsigned int *) CDEV_BITMAP_ADDR(current_interrupt_line));

    //restituisce il n. di terminale che ha generato l'interrupt
    int device_no = 0;
    while (device_no < 8 && !(p & 1)) {
        device_no++;
        p <<= 1;
    }

    assert(device_no < 8);

    acknowledge(EXT_IL_INDEX(current_interrupt_line), device_no);

    tprint("io_h should not arrive here!\n");
    PANIC();
}

void acknowledge_dtpe(uintptr_t line, uintptr_t device);
void acknowledge_terminal(uintptr_t line, uintptr_t device);

static inline void acknowledge(unsigned int line, unsigned int device)
{
    assert(soft_blocked_thread[line][device] != NULL);

    switch (line) {
        case EXT_IL_INDEX(IL_TERMINAL):
            acknowledge_terminal(line, device);
            break;
        case EXT_IL_INDEX(IL_DISK):
        case EXT_IL_INDEX(IL_TAPE):
        case EXT_IL_INDEX(IL_PRINTER):
        case EXT_IL_INDEX(IL_ETHERNET):
            acknowledge_dtpe(line, device);
            break;
        default:
            tprint("io_h - acknowledge: wrong device");
            PANIC();
    }

    soft_blocked_thread[line][device] = NULL;
    soft_block_count--;

    if (current_thread == NULL)
    // se il messaggio è stato inviato mentre il processore era idle
        // chiamiamo lo scheduler per schedulare il processo svegliato
        scheduler();
    else
        LDST((state_t *) INT_OLDAREA);
}

void acknowledge_dtpe(uintptr_t line, uintptr_t device)
{
    unsigned int device_status = *((unsigned int *) DEV_REG_ADDR(line, device) + 0x0);
    *((unsigned int *) DEV_REG_ADDR(line, device) + 0x4) = DEV_C_ACK;

    // Sblocchiamo il processo in attesa a nome dell'SSI
    send(soft_blocked_thread[line][device], SSI, device_status);
}

// TODO: completare con i due casi
void acknowledge_terminal(uintptr_t line, uintptr_t device)
{
    unsigned int transmission_status = *((unsigned int *) (DEV_REG_ADDR(IL_TERMINAL, device) + 0x8));
    unsigned int receive_status      = *((unsigned int *) (DEV_REG_ADDR(IL_TERMINAL, device) + 0x0));

    if (receive_status == DEV_S_READY) {
        *((unsigned int *) (DEV_REG_ADDR(IL_TERMINAL, device) + 0xC)) = DEV_C_ACK;
        send(soft_blocked_thread[line][device], SSI, transmission_status);
    } else if (transmission_status == DEV_S_READY) {
        *((unsigned int *) (DEV_REG_ADDR(IL_TERMINAL, device) + 0x4)) = DEV_C_ACK;
        send(soft_blocked_thread[line][device], SSI, receive_status);
    }
}
