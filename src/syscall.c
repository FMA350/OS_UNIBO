#include <mikabooq.h>
#include <nucleus.h>
#include <syslib.h>

extern struct list_head readyq;

// sentinella della coda dei processi in attesa di ricevere un messaggio
LIST_HEAD(blockedq);

extern struct tcb_t *current_thread;


// TODO: cosa fare se il thread si reincarna?
static inline send(struct tcb_t *dest, uintptr_t msg){
    switch (dest->t_status) {
        case T_STATUS_READY:
        /* Se il thread destinazione non è in attesa di un messaggio */
            if (msgq_add(current_thread, dest, msg) == -1)
            /* Se i messaggi disponibili sono finiti */
            // TODO: trovare una soluzione migliore
                PANIC();
            break;
        case T_STATUS_W4MSG:
        /* Se il thread destinazione è in attesa di un messaggio */
            if (dest->t_wait4sender == current_thread) {
            /* il thread di destinazione aspetta un messaggio da
                parte del processo corrente */

                // il messaggio è consegnato con priorità
                if (msgq_add_head(current_thread, dest, msg) == -1)
                /* Se i messaggi disponibili sono finiti */
                // TODO: trovare una soluzione migliore
                    PANIC();

                dest->t_status = T_STATUS_READY;
                dest->t_wait4sender = NULL;
                // dest è rimosso dai processi in attesa
                thread_outqueue(dest);
                // e reinserito nella coda ready
                thread_enqueue(dest, &readyq);

            } else if (msgq_add(current_thread, dest, msg) == -1)
            /* Se i messaggi disponibili sono finiti */
            // TODO: trovare una soluzione migliore
                PANIC();

            break;
        case T_STATUS_NONE:
            // TODO: controlli nel caso il thread fosse terminato
            break;
    }

    LDST((state_t *) SYSBK_OLDAREA);
}

static inline recv(struct tcb_t *src, uintptr_t *pmsg){

    if (msgq_get(&src, current_thread, pmsg) == 0)
    /* il messaggio cercato si trova nella coda */
        LDST((state_t *) SYSBK_OLDAREA);
    else {
        // salvataggio stato del processore
        current_thread->t_s = *((state_t *) INT_OLDAREA);
        // Quando questo thread riprenderà l'esecuzione chiamare di nuovo la receive
        current_thread->t_s.pc -= 4;

        // changing thread status
        current_thread->t_status = T_STATUS_W4MSG;
        current_thread->t_wait4sender = src;

        // Inserimento del processo nella coda dei processi in attesa di messaggi
        thread_enqueue(current_thread, &blockedq);
        scheduler();
    }
}

void syscall_h(){
    // copiare old_state in thread->t_s

    switch (((state_t *) SYSBK_OLDAREA)->a1) {
        case SYS_ERR:
            // Segnalazione di errore
            break;
        case SYS_SEND:
            send((struct tcb_t *) ((state_t *) SYSBK_OLDAREA)->a2, ((state_t *) SYSBK_OLDAREA)->a3);
        case SYS_RECV:
            recv((struct tcb_t *) ((state_t *) SYSBK_OLDAREA)->a2, (uintptr_t *) ((state_t *) SYSBK_OLDAREA)->a3);
        default:
            /* system call non 1 o 2 vengono trasformate in messaggi al thread
            definito tramite SETSYSMGR se esiste altrimenti msg SETPGMMGR se
            esiste altrimenti TERMINATE_THREAD  */
            break;
    }

    LDST((state_t *) SYSBK_OLDAREA);
}
