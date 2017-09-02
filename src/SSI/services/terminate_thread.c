/* terminate the thread. thread != NULL
   the process is also removed from the scheduling queue it's in (device queue also)*/
inline void __terminate_thread_s(struct tcb_t *thread)
{
    //tprint("__terminate_thread_s started\n");
    while (!list_empty(&thread->t_msgq))
    //cancello tutti i messaggi se ce ne sono
        msg_free(msg_qhead(&thread->t_msgq));

    // sbloccare i processi in attesa di messaggi del thread da terminare
    struct tcb_t *to_resume;
    // tprintf("coda t_wait4me --> %p\n", &thread->t_wait4me);
    // tprintf("puntatori t_wait4me: next --> %p, prev --> %p\n", thread->t_wait4me.next, thread->t_wait4me.prev);
    // tprintf("la coda e' vuota? --> %d\n", list_empty(&thread->t_wait4me));

    while (to_resume = thread_qhead(&thread->t_wait4me)) {
        // tprintf("resuming thread - %p\n", to_resume);
        to_resume->errno = 1; //setto errno = 1 per dire ai processi che si aspettavano un msg che il mittente e' morto
        resume_thread(to_resume, NULL, 0);
    }
    // tprint("waiting threads resumed\n");

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
inline void terminate_thread_s(struct tcb_t *thread)
{
    //tprint("terminate_thread_s started\n");

    if(list_is_only(&thread->t_next, &get_processid_s(thread)->p_threads)) {
    // se è l'unico thread del processo
        // terminiamo l'intero processo
        terminate_process_s(thread);
    } else {
    // Se il ha fratelli
        // terminiamo unicamente questo thread
        __terminate_thread_s(thread);
    }

    //tprint("terminate_thread_s ended\n\n");
}
