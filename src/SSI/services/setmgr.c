/* *send_back è 1 se bisogna spedire una risposta al mittente, cioè il processo non è stato terminato */
inline struct tcb_t *
__setmgr(struct tcb_t *new_mgr, struct tcb_t *applicant,
        struct tcb_t **mgr, int *send_back)
{
    if (!new_mgr) {
        *send_back = 1;
        return NULL;
    }
    if(*mgr) {
    // se il manager è già settato
        *send_back = 0;
        terminate_process_s(applicant);
        return NULL;
    } else {
    // se il manager non è mai stato settato
        *send_back = 1;
        return *mgr = new_mgr;
    }
}

struct tcb_t *setpgmmgr_s(struct tcb_t *new_mgr, struct tcb_t *applicant, int *send_back)
{
    return __setmgr(new_mgr, applicant, &applicant->t_pcb->pgm_mgr, send_back);
}

struct tcb_t *settlbmgr_s(struct tcb_t *new_mgr, struct tcb_t *applicant, int *send_back)
{
    return __setmgr(new_mgr, applicant, &applicant->t_pcb->tlb_mgr, send_back);
}

struct tcb_t *setsysmgr_s(struct tcb_t *new_mgr, struct tcb_t *applicant, int *send_back)
{
    return __setmgr(new_mgr, applicant, &applicant->t_pcb->sys_mgr, send_back);
}
