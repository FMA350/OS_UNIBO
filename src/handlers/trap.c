/*
 * I manager ricevono messaggi che hanno come sender il descrittore del thread
 * che ha causato la trap ed un puntatore al suo stato come payload del messaggio.
 */

#include <nucleus.h>
#include <scheduler.h>
#include <uARMconst.h>

extern struct list_head blockedq;

static inline void __trap_h(struct tcb_t *mgr, state_t *oldarea)
{
    if (mgr) {
    // se il manager è stato settato (non è NULL)
        current_thread->t_s = *oldarea; // salvataggio stato del processore
        // the instruction that raised the trap must be repeated
        current_thread->t_s.pc -= 4;
        // si invia un messaggio al manager con lo stato del processo che ha generato la trap
		msgsend(mgr, (unsigned int) &current_thread->t_s);

        // blocchiamo il thread
        thread_enqueue(current_thread, &blockedq);
        /* HACK:
         * Blocchiamo il thread senza fare la receive perché essa salverebbe
         * lo stato del processore, sovrascrivendo così lo stato del processore
         * nel momento in cui la trap è stata generata.
         * I manager dopo aver intrapreso le operazioni opportune reinseriranno
         * il thread nella coda ready.
         */
        scheduler();
    } else {
    // il processo deve essere terminato
        terminate_process();
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
