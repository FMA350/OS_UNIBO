

extern struct list_head blockedq;


static inline void __trap_h(struct tcb_t *mgr)
{
    tprint("mng_name started\n");

    if (mgr) {
    // se il manager è settato
        int exc_cause = getCAUSE();
		msgsend(mgr, (unsigned int) exc_cause);
    } else {
        struct {
			uintptr_t reqtag;
		} req = {TERMINATE_PROCESS};
		msgsend(SSI, &req);
    }

    thread_enqueue(current_thread, &blockedq);
	scheduler();
}


void pgmtrap_h()
{
    current_thread->t_s = *((state_t *) PGMTRAP_OLDAREA);
    __trap_h(current_thread->t_pcb->pgm_mgr);
}

void tlbtrap_h()
{
    current_thread->t_s = *((state_t *) TLBTRAP_OLDAREA);
	__trap_h(current_thread->t_pcb->tlb_mgr);
}
