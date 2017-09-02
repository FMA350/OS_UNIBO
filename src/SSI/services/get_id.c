struct pcb_t *
get_processid_s(const struct tcb_t *thread)
{
    return thread->t_pcb;
}

struct pcb_t *
get_parentprocid_s(const struct pcb_t *proc)
{
    return proc->p_parent;
}

struct tcb_t *
get_mythreadid_s(const struct tcb_t *thread)
{
    return thread;
}
