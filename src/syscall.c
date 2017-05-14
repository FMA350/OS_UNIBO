#include <mikabooq.h>
#include <nucleus.h>
#include <syslib.h>
#include <scheduler.h>
#include <syscall.h>


// sentinella della coda dei processi in attesa di ricevere un messaggio
static LIST_HEAD(blockedq);

#define ST_RVAL(RVAL)   \
    (((state_t *) SYSBK_OLDAREA)->a1 = (unsigned int) (RVAL))

#define SYSCALL_ARG(N)  \
    (((state_t *) SYSBK_OLDAREA)->a ## N)

// il chiamante è in kernel mode
// TODO: check
#define IS_KERNEL_MODE  \
    (((state_t *) SYSBK_OLDAREA)->cpsr & STATUS_SYS_MODE == STATUS_SYS_MODE)


static inline void DELIVER_MSG(struct tcb_t *dest, struct tcb_t *sender, uintptr_t msg) {
    // TODO: forse è da modificare con gli interrupt dei devices msgq_add
    if (msgq_add(sender, dest, msg) == 0)
    /* Se la consegna del messaggio è andata a buon fine */
        ST_RVAL(SEND_SUCCESS);
    else
    /* Se i messaggi disponibili sono finiti */
        ST_RVAL(SEND_FAILURE);
}

/*
 * This function deliver the message directly, without passing from the message
 * queue of thread dest
 *
 * Preconditions:
 * dest is currently in the blocked queue. It's waiting for a message from the
 * thread that calls this function (current thread) or from any thread.
 */
static inline void DELIVER_DIRECTLY(struct tcb_t *dest, struct tcb_t *recv_rval, uintptr_t msg) {
    dest->t_s.a1 = (unsigned int) recv_rval;
    // check that recv has been called with pmsg != NULL
    if (((uintptr_t *) (dest->t_s.a3)) != NULL)
        *((uintptr_t *) (dest->t_s.a3)) = msg;
}


inline void resume_thread(struct tcb_t *resuming, struct tcb_t *recv_rval, uintptr_t msg) {

    // il messaggio è consegnato con priorità
    DELIVER_DIRECTLY(resuming, recv_rval, msg);

    resuming->t_status = T_STATUS_READY;
    resuming->t_wait4sender = NULL;

    // dest è rimosso dai processi in attesa
    thread_outqueue(resuming);
    // e reinserito nella coda ready
    thread_enqueue(resuming, &readyq);
}

// TODO: cosa fare se il thread si reincarna?
extern void send(struct tcb_t *dest, struct tcb_t *sender, uintptr_t msg){
    tprint("send starting\n");

    switch (dest->t_status) {
        case T_STATUS_READY:
        /* Se il thread destinazione non è in attesa di un messaggio */

            DELIVER_MSG(dest, sender, msg);

            break;
        case T_STATUS_W4MSG:
        /* Se il thread destinazione è in attesa di un messaggio */

            if (dest->t_wait4sender == sender || dest->t_wait4sender == NULL) {
            /* il thread di destinazione aspetta un messaggio da
                parte del processo corrente o da qualsiasi processo (non ha messaggi) */

                //se effettivamente dest stava aspettando un messaggio da me(current thread) devo eliminare dst dalla
                //mia lista di thread che aspettano messaggi da me
                if (dest->t_wait4sender)
                    wait4thread_del(dest);

				resume_thread(dest, sender, msg);

                ST_RVAL(SEND_SUCCESS);
            }
            else
                DELIVER_MSG(dest, sender, msg);
            break;
        case T_STATUS_NONE:

            ST_RVAL(SEND_FAILURE);
            break;
    }

    LDST((state_t *) SYSBK_OLDAREA); //segment error!!!
    //tprint("send a\n");

}

static inline void recv(struct tcb_t *src, uintptr_t *pmsg){

    if (msgq_get(&src, current_thread, pmsg) == 0) {    // in src viene memorizzato il mittente
    /* caso non bloccante: il messaggio cercato si trova nella coda */
        ST_RVAL(src);
        LDST((state_t *) SYSBK_OLDAREA);
    } else {
    /* caso bloccante */
        // salvataggio stato del processore
        current_thread->t_s = *((state_t *) SYSBK_OLDAREA);

        // changing thread status
        current_thread->t_status = T_STATUS_W4MSG;
        current_thread->t_wait4sender = src;

        // se current_thread aspetta un messaggio da un thread preciso
        if (src)
        //aggiunge il processo corrente alla lista dei processi che aspettano src (di src)
            wait4thread_add(src, current_thread);
        // Inserimento del processo nella coda dei processi in attesa di messaggi
        thread_enqueue(current_thread, &blockedq);
        scheduler();
    }
}

/*******************************************************************************/
/*
 * These two functions are wrappers for send and recv
 * send and recv are called only if the calling thread is in kernel mode,
 * otherwise an error value is returned and the error number is set
 *
 * Note: these functions never return control to the caller
 */

static inline void send_kernel(struct tcb_t *dest, uintptr_t msg) {
    if (IS_KERNEL_MODE)
        send(dest, current_thread, msg);
    else {
        ST_RVAL(SEND_FAILURE);
        // TODO: set error number

        LDST((state_t *) SYSBK_OLDAREA);
    }
}

static inline void recv_kernel(struct tcb_t *src, uintptr_t *pmsg) {
    if (IS_KERNEL_MODE)
        recv(src, pmsg);
    else {
        ST_RVAL(RECV_FAILURE);
        // TODO: set error number

        LDST((state_t *) SYSBK_OLDAREA);
    }
}

/*******************************************************************************/
// Syscall != 0, 1, 2

/* system call non 1 o 2 vengono trasformate in messaggi al thread
definito tramite SETSYSMGR se esiste altrimenti msg SETPGMMGR se
esiste altrimenti TERMINATE_THREAD  */


static inline void syscall_other(uintptr_t msg) {
    // se è definito un thread tramite SETSYSMGR
        // send(/*Thread*/, );
    // altrimenti se esiste un thread definito tramite SETPGMMGR
        // send(/*Thread*/, );
    // altrimenti TERMINATE_THREAD
        // send(/* SSI */, TERMINATE_THREAD);
}

/*******************************************************************************/
// Syscall Handler


void syscall_h(){
    // copiare old_state in thread->t_s
    BREAKPOINT();
    switch (SYSCALL_ARG(1)) {
        case SYS_SEND:
            send_kernel((struct tcb_t *) SYSCALL_ARG(2), SYSCALL_ARG(3));
        case SYS_RECV:
            recv_kernel((struct tcb_t *) SYSCALL_ARG(2), (uintptr_t *) SYSCALL_ARG(3));
        default:
            syscall_other(SYSCALL_ARG(3));
    }
}
