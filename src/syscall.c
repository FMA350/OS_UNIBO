#include <mikabooq.h>
#include <nucleus.h>
#include <syslib.h>

extern struct list_head readyq;

// sentinella della coda dei processi in attesa di ricevere un messaggio
LIST_HEAD(blockedq);

extern struct tcb_t *current_thread;



#define SEND_SUCCESS    0

#define SEND_FAILURE    -1


#define SYSCALL_ARG(N)  \
    (((state_t *) SYSBK_OLDAREA)->a ## N)


#define ST_RVAL(RVAL)   \
    ((state_t *) SYSBK_OLDAREA)->a1 = (unsigned int) (RVAL)


/* DELIVERING_FUN == msgq_add, msgq_add_tail */
#define DELIVER_MSG(DELIVERING_FUN, DEST, MSG) {                    \
    if (DELIVERING_FUN(current_thread, DEST, MSG) == 0)             \
    /* Se la consegna del messaggio è andata a buon fine */         \
        ST_RVAL(SEND_SUCCESS);                                      \
    else                                                            \
    /* Se i messaggi disponibili sono finiti */                     \
        ST_RVAL(SEND_FAILURE);                                      \
    }


// TODO: is blockedq necessary?

// send ritorna 0 in caso di successo, 1 in caso di fallimento
// TODO: cosa fare se il thread si reincarna?
static inline send(struct tcb_t *dest, uintptr_t msg){
    switch (dest->t_status) {
        case T_STATUS_READY:
        /* Se il thread destinazione non è in attesa di un messaggio */
            DELIVER_MSG(msgq_add, dest, msg);
            break;
        case T_STATUS_W4MSG:
        /* Se il thread destinazione è in attesa di un messaggio */
            if (dest->t_wait4sender == current_thread || dest->t_wait4sender == NULL) {
            /* il thread di destinazione aspetta un messaggio da
                parte del processo corrente o da qualsiasi processo (non ha messaggi) */

                // il messaggio è consegnato con priorità
                DELIVER_MSG(msgq_add_head, dest, msg);

                dest->t_status = T_STATUS_READY;
                dest->t_wait4sender = NULL; // non necessario
                // dest è rimosso dai processi in attesa
                thread_outqueue(dest);
                // e reinserito nella coda ready
                thread_enqueue(dest, &readyq);
            }
            else
                DELIVER_MSG(msgq_add, dest, msg);

            break;
        case T_STATUS_NONE:
            ST_RVAL(SEND_FAILURE);
    }

    LDST((state_t *) SYSBK_OLDAREA);
}

static inline recv(struct tcb_t *src, uintptr_t *pmsg){

    if (msgq_get(&src, current_thread, pmsg) == 0) {    // in src viene memorizzato il mittente
    /* caso non bloccante: il messaggio cercato si trova nella coda */
        ST_RVAL(src);
        LDST((state_t *) SYSBK_OLDAREA);
    } else {
    /* caso bloccante */
        // salvataggio stato del processore
        current_thread->t_s = *((state_t *) SYSBK_OLDAREA);
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
    BREAKPOINT();
    switch (SYSCALL_ARG(1)) {
        case SYS_ERR:
            // Segnalazione di errore
            break;
        case SYS_SEND:
            send((struct tcb_t *) SYSCALL_ARG(2), SYSCALL_ARG(3));
        case SYS_RECV:
            recv((struct tcb_t *) SYSCALL_ARG(2), (uintptr_t *) SYSCALL_ARG(3));
        default:
            /* system call non 1 o 2 vengono trasformate in messaggi al thread
            definito tramite SETSYSMGR se esiste altrimenti msg SETPGMMGR se
            esiste altrimenti TERMINATE_THREAD  */
            break;
    }

    // LDST((state_t *) SYSBK_OLDAREA);
}
