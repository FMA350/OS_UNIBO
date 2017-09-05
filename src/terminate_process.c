#include <mikabooq.h>
#include <ssi.h>

extern struct pcb_t *get_processid_s(const struct tcb_t *thread);
extern void __terminate_thread_s(struct tcb_t *thread);

/* Cleans the eventual messages sent to managers and SSI
   Preconditions: the thread is waiting

   sys_mgr e pgm_mgr sono gli unici thread di sistema oltre all'SSI
   nei confronti dei quali è possibile fare la recv (syscall_other)
   */
// FIXME: probabilmente non è corretto
static inline void clean_sys_msg(struct tcb_t *terminating)
{
    if (terminating->t_wait4sender == SSI) {
        // msgq_get should always succeed
        msgq_get(&terminating, SSI, NULL);
    } else if (terminating->t_wait4sender == (get_processid_s(terminating)->sys_mgr)) {
         msgq_get(&terminating, get_processid_s(terminating)->sys_mgr, NULL);
    } else if (terminating->t_wait4sender == (get_processid_s(terminating)->pgm_mgr)) {
        msgq_get(&terminating, get_processid_s(terminating)->sys_mgr, NULL);
    }
}

/*
 * Terminate the process and all his progeny
 *
 * Preconditions: process != NULL, applicant is the requestor of the service
 *
 */
static void __terminate_process_s(struct pcb_t *proc, struct tcb_t *applicant)
{
    // TODO: eliminare dalla coda dei messaggi dell'SSI eventuali messaggi
    // provenienti dai thread dei processi figli che saranno terminati

    // eliminiamo tutti i thread
    struct tcb_t *thread_term;
    while (thread_term = proc_firstthread(proc)) {
        // terminate thread changes the structure
        if (thread_term != applicant && thread_term->t_status == T_STATUS_W4MSG) {
        // Se il thread non è quello che ha richiesto il servizio e sta aspettando
            clean_sys_msg(thread_term);
        }
        __terminate_thread_s(thread_term);
    }

    // eliminiamo i figli del processo ricorsivamente
    struct pcb_t *proc_term;
    while (proc_term = proc_firstchild(proc)) {
        __terminate_process_s(proc_term, NULL);
    }
    // eliminiamo il processo

    proc_delete(proc);
}

void terminate_process_s(struct tcb_t *applicant)
{
    __terminate_process_s(get_processid_s(applicant), applicant);
}
