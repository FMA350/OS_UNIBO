/*
 * I manager ricevono messaggi che hanno come sender il descrittore del thread
 * che ha causato la trap ed un puntatore al suo stato come payload del messaggio.
 */
#include <nucleus.h>
#include <scheduler.h>
#include <uARMconst.h>
#include <handlers.h>
#include <syslib.h>

extern struct list_head blockedq;

static inline void __trap_h(struct tcb_t *mgr, state_t *oldarea)
{
    if (mgr) {
    // se il manager è stato settato (non è NULL)
        // salvataggio stato del processore
        current_thread->t_s = *oldarea;
        // the instruction that raised the trap must be repeated
        current_thread->t_s.pc -= 4; //non serve, se c'e il mgr ci pensa lui!
        // si invia un messaggio al manager con lo stato del thread che ha generato la trap
        int rval = send(mgr, current_thread, (uintptr_t) &current_thread->t_s);
        assert(rval == SEND_SUCCESS);
        /* HACK:
         * Blocchiamo il thread senza fare la receive perché essa salverebbe
         * lo stato del processore, sovrascrivendo così lo stato del processore
         * nel momento in cui la trap è stata generata.
        */
        current_thread->t_status = T_STATUS_W4MSG;
        current_thread->t_wait4sender = mgr;
        move_thread(current_thread, &mgr->t_wait4me);
        scheduler();
    } else {
    // il processo deve essere terminato
        terminate_thread_s(current_thread);
        scheduler();
    }

    tprint("__trap_h should not arrive here!\n");
    PANIC();
}

void pgmtrap_h(void)
{
    __trap_h(current_thread->t_pcb->pgm_mgr, (state_t *) PGMTRAP_OLDAREA);
}

void tlbtrap_h(void)
{
    __trap_h(current_thread->t_pcb->tlb_mgr, (state_t *) TLB_OLDAREA);
}
