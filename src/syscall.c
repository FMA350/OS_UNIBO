
// sentinella della coda dei processi in attesa di ricevere un messaggio
LIST_HEAD(blockedq);

static inline send(struct tcb_t *dest, uintptr_t msg){

}

static inline recv(struct tcb_t *src, uintptr_t *pmsg){

}

void syscall_h(){
    // copiare old_state in thread->t_s

    switch (((state_t *) SYSBK_OLDAREA)->a1) {
        case 0:
            // Segnalazione di errore
        case 1:
            send(((state_t *) SYSBK_OLDAREA)->a2, ((state_t *) SYSBK_OLDAREA)->a3);
        case 2:
            recv(((state_t *) SYSBK_OLDAREA)->a2, ((state_t *) SYSBK_OLDAREA)->a3);
        default:
            NULL;
            /* system call non 1 o 2 vengono trasformate in messaggi al thread
            definito tramite SETSYSMGR se esiste altrimenti msg SETPGMMGR se
            esiste altrimenti TERMINATE_THREAD  */
    }

    LDST((state_t *) SYSBK_OLDAREA);
}
