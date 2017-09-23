/*
 * I manager ricevono messaggi che hanno come sender il descrittore del thread
 * che ha causato la trap ed un puntatore al suo stato come payload del messaggio.
 */
#include <nucleus.h>
#include <scheduler.h>
#include <uARMconst.h>


extern struct list_head blockedq;

int trap_flag = 0;

static inline void __trap_h(struct tcb_t *mgr, state_t *oldarea)
{
    if (mgr) {
    // se il manager è stato settato (non è NULL)
        tprint(">>> mgr\n");
        // salvataggio stato del processore
        current_thread->t_s = *oldarea;
        // the instruction that raised the trap must be repeated
        //current_thread->t_s.pc -= 4; //non serve, se c'e il mgr ci pensa lui!
        // si invia un messaggio al manager con lo stato del thread che ha generato la trap
        msgsend(mgr, &current_thread->t_s);
        /* HACK:
         * Blocchiamo il thread senza fare la receive perché essa salverebbe
         * lo stato del processore, sovrascrivendo così lo stato del processore
         * nel momento in cui la trap è stata generata.
        */
        current_thread->t_status = T_STATUS_W4MSG;
        current_thread->t_wait4sender = mgr;
        tprint(">>> about to enqueue\n");
        trap_flag = 1;
        thread_enqueue(current_thread, &mgr->t_wait4me);
        trap_flag = 0;
        tprint(">>> abount to call scheduler\n");
        scheduler();
    } else {
    // il processo deve essere terminato
        tprint(">>> mgr not specified\n");
        terminate_thread_s(current_thread);

        tprint(">>> abount to call scheduler\n");
        scheduler();
    }

    tprint("__trap_h should not arrive here!\n");
    PANIC();
}

void pgmtrap_h(void)
{
    trap_flag = 0;
    // ((state_t *) PGMTRAP_OLDAREA)->CP15_Cause = EXC_RESERVEDINSTR;
    tprintf("=== pgmtrap_h started ===\n");
    tprintf("pgmtrap_h: cause == %d\n", (int) CAUSE_EXCCODE_GET(((state_t *) PGMTRAP_OLDAREA)->CP15_Cause));
    // ((state_t *) PGMTRAP_OLDAREA)->CP15_Cause = CAUSE_EXCCODE_SET(((state_t *) PGMTRAP_OLDAREA)->CP15_Cause, EXC_RESERVEDINSTR);
    __trap_h(current_thread->t_pcb->pgm_mgr, (state_t *) PGMTRAP_OLDAREA);
}

void tlbtrap_h(void)
{
    trap_flag = 0;
    tprintf("=== tlbtrap_h started ===\n");
    tprintf("tlbtrap_h: cause == %d\n", (int) CAUSE_EXCCODE_GET(((state_t *) TLB_OLDAREA)->CP15_Cause));
    __trap_h(current_thread->t_pcb->tlb_mgr, (state_t *) TLB_OLDAREA);
}
