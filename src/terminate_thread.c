#include <mikabooq.h>
#include <scheduler.h>

extern struct pcb_t *get_processid_s(const struct tcb_t *thread);

/* terminate the thread. thread != NULL
   the process is also removed from the scheduling queue it's in (device queue also)*/
void __terminate_thread_s(struct tcb_t *thread)
{
    while (!list_empty(&thread->t_msgq))
    //cancello tutti i messaggi se ce ne sono
        msg_free(msg_qhead(&thread->t_msgq));

    // sbloccare i processi in attesa di messaggi del thread da terminare
    struct tcb_t *to_resume;

    while (to_resume = thread_qhead(&thread->t_wait4me)) {
        to_resume->errno = 1; //setto errno = 1 per dire ai processi che si aspettavano un msg che il mittente e' morto
        resume_thread(to_resume, NULL, 0);
    }

    int err = thread_free(thread);
    // if (err == -1) {
    //     tprint("ERROR - msgq\n");
    //     HALT();
    // } else if (err == -2) {
    //     tprint("ERROR - wait4me\n");
    //     HALT();
    // }
    // tprint("ending __terminate_thread_s\n");

    thread_count--;
}

/* terminate the thread and, if it's the last one, the process too */
void terminate_thread_s(struct tcb_t *thread)
{
    if(list_is_only(&thread->t_next, &get_processid_s(thread)->p_threads)) {
    // se è l'unico thread del processo
        // terminiamo l'intero processo
        terminate_process_s(thread);
    } else {
    // Se il ha fratelli
        // terminiamo unicamente questo thread
        __terminate_thread_s(thread);
    }
}
