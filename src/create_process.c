#include <mikabooq.h>

extern inline struct tcb_t *__create_thread_s(const state_t *initial_state, struct pcb_t *proc);

struct tcb_t *create_process_s(const state_t *initial_state, struct tcb_t *applicant)
{
    struct pcb_t *new_process = proc_alloc(get_processid_s(applicant));

    if(new_process == NULL) {
        return NULL;
    }

    struct tcb_t *first_thread = __create_thread_s(initial_state, new_process);

    if(!first_thread){
        // throw an error, no more space availeable
        proc_delete(new_process);
        return NULL;
    }

    return first_thread;
}
