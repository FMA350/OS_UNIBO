
inline struct tcb_t *__create_thread_s(const state_t *initial_state, struct pcb_t *proc)
{
    struct tcb_t *new_thread = thread_alloc(proc);
    if(!new_thread) {
        return NULL;
    }
    new_thread->t_s = *initial_state; //memcpy
    //tprintf("current: %p, %p, %p \n", current_thread,  thread_qhead(&readyq),  thread_qhead(&blockedq));
    thread_enqueue(new_thread, &readyq);
    thread_count++;
    return new_thread;
}


extern struct pcb_t * get_processid_s(const struct tcb_t *thread);

struct tcb_t * create_thread_s(const state_t *initial_state, struct tcb_t *applicant)
{
    return __create_thread_s(initial_state, get_processid_s(applicant));
}
