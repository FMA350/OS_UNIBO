#include <mikabooq.h>
#include <nucleus.h>
#include <syslib.h>
#include <scheduler.h>

#include "handlers.h"
#include "accounting.h"

extern unsigned int cyclesUsed;


static inline void store_return_value(unsigned int rval)
{
    ((state_t *) SYSBK_OLDAREA)->a1 = rval;
}

static inline int
deliver_msg(struct tcb_t *dest, struct tcb_t *sender, uintptr_t msg)
{
    // TODO: forse è da modificare con gli interrupt dei devices msgq_add
    //       mnalli - perché?

    if (msgq_add(sender, dest, msg) == 0)
    /* Se la consegna del messaggio è andata a buon fine */
        return SEND_SUCCESS;
    else
    /* Se i messaggi disponibili sono finiti */
        return SEND_FAILURE;
}

/*
 * This function deliver the message directly, without passing from the message
 * queue of thread dest
 *
 * Preconditions:
 * dest is currently blocked. It's waiting for a message from the
 * thread that calls this function (current thread) or from any thread.
 */
static inline void
deliver_directly(struct tcb_t *dest, unsigned int recv_rval, uintptr_t msg)
{
    // a1 is the register where the return value of functions is stored
        if (current_thread != dest->t_pcb->sys_mgr) //TODO: non mi viene in mente niente di meglio...
            dest->t_s.a1 = recv_rval;
        else
            dest->t_s.a1 = dest->t_s.a4;

    // check that recv has been called with pmsg != NULL
    if (((uintptr_t *) (dest->t_s.a3)) != NULL){
        *((uintptr_t *) (dest->t_s.a3)) = msg;
    }
}

/*
 * Resume a thread blocked while receiving delivering msg and returning recv_rval
 * to the thread
 * Preconditions: resuming is blocked. resuming is in his queue.
 */
void resume_thread(struct tcb_t *resuming, unsigned int recv_rval, uintptr_t msg)
{
    // il messaggio è consegnato con priorità
    deliver_directly(resuming, recv_rval, msg);

    resuming->t_status = T_STATUS_READY;
    resuming->t_wait4sender = NULL;
    // dest è rimosso dai processi in attesa (blockedq se stava aspettando da
    // chiunque sender->t_wait4me se stava aspettando da sender)
    thread_outqueue(resuming);
    // e reinserito nella coda ready
    thread_enqueue(resuming, &readyq);

}

/* La send non interagisce con lo stato nel sender nè con l'oldarea */
unsigned int send(struct tcb_t *dest, struct tcb_t *sender, uintptr_t msg)
{
    //tprintf("%p sends to %p\n", sender, dest);
    switch (dest->t_status) {
        case T_STATUS_READY:
        /* Se il thread destinazione non è in attesa di un messaggio */
            return deliver_msg(dest, sender, msg);
        case T_STATUS_W4MSG:
        /* Se il thread destinazione è in attesa di un messaggio */
            if (dest->t_wait4sender == sender || dest->t_wait4sender == NULL) {
            /*
             * il thread di destinazione aspetta un messaggio da:
             * Processo corrente
             * Qualsiasi processo (in tal caso non ha messaggi)
             */
                resume_thread(dest, (unsigned int) sender, msg);
                return SEND_SUCCESS;

            } else {
            /* dest sta aspettando un messaggio da qualcun'altro */
                return deliver_msg(dest, sender, msg);
            }
        case T_STATUS_NONE:
            return SEND_FAILURE;
            // FIXME: SEND_FAILURE == -1, send return unsigned int
    }
}

static inline void recv(struct tcb_t *sender, uintptr_t *pmsg)
{
    if (msgq_get(&sender, current_thread, pmsg) == 0) {    // in src viene memorizzato il mittente
    /* caso non bloccante: il messaggio cercato si trova nella coda */
        store_return_value((unsigned int) sender);
        LDST((state_t *) SYSBK_OLDAREA);
    } else {
    /* caso bloccante */
    /* la msgq_get non ha trovato nessun messaggio -> sender non è stato modificato
        nemmeno nel caso in cui il chiamante abbia passato NULL come sender (chiunque) */

        timeSliceLeft = getTIMER();
        if((timeSliceLeft > 0) && (timeSliceLeft < TICKS_PER_TIME_SLICE)){
            cyclesUsed = TICKS_PER_TIME_SLICE - timeSliceLeft;
        }
        else{
            cyclesUsed = TICKS_PER_TIME_SLICE;
        }


        // salvataggio stato del processore
        current_thread->t_s = *((state_t *) SYSBK_OLDAREA);
        // cambiamento dello stato di attesa del thread
        current_thread->t_status = T_STATUS_W4MSG;
        current_thread->t_wait4sender = sender;

        current_thread->run_time += cyclesUsed; //cycles
        update_clock(cyclesUsed);

        if (sender) {
        // il thread si blocca aspettando da sender
            // aggiunge il processo corrente alla lista dei processi che aspettano sender (di sender)
            thread_enqueue(current_thread, &sender->t_wait4me);
            //tprintf("%p t_wait4me head = %p\n",sender, thread_qhead(&sender->t_wait4me));
        } else {
        // il thread si blocca aspettando da chiunque
            // Inserimento del processo nella coda dei processi in attesa di messaggi da chiunque
            thread_enqueue(current_thread, &blockedq);
            // tprintf("current: %p, blockedqH: %p, ready:%p\n", current_thread, thread_qhead(&blockedq), thread_qhead(&readyq));
        }
        scheduler();
    }
}

/*******************************************************************************/

// solleva una trap
static inline void raise_reserved_instruction(void)
{
    //tprint("raise_reserved_instruction\n");
    *((state_t *) PGMTRAP_OLDAREA) = *((state_t *) SYSBK_OLDAREA);
    ((state_t *) PGMTRAP_OLDAREA)->CP15_Cause = EXC_RESERVEDINSTR;
    pgmtrap_h();
}

// il chiamante è in kernel mode
// TODO: check
#define IS_KERNEL_MODE  \
    (((state_t *) SYSBK_OLDAREA)->cpsr & STATUS_SYS_MODE == STATUS_SYS_MODE)

/*
 * These two functions are wrappers for send and recv
 * send and recv are called only if the calling thread is in kernel mode,
 * otherwise an error value is returned and the error number is set
 *
 * Note: these functions never return control to the caller
 */

static inline void send_kernel(struct tcb_t *dest, uintptr_t msg)
{
    if (IS_KERNEL_MODE) {
        store_return_value(send(dest, current_thread, msg));
        LDST((state_t *) SYSBK_OLDAREA);
    }
    else
        raise_reserved_instruction();
}

static inline void recv_kernel(struct tcb_t *src, uintptr_t *pmsg)
{
    if (IS_KERNEL_MODE)
        recv(src, pmsg);
    else
        raise_reserved_instruction();
}

/*******************************************************************************/

/*
 * system call non 0, 1 o 2 vengono trasformate in messaggi al thread
 * definito tramite SETSYSMGR se esiste altrimenti msg SETPGMMGR se
 * esiste altrimenti TERMINATE_THREAD.
 */

static inline void syscall_other(void)
{
    // process of the current thread
    struct pcb_t *current_process = current_thread->t_pcb;
    if (current_process->sys_mgr) {
        current_thread->t_s = *((state_t *) SYSBK_OLDAREA); // salvataggio stato del processore
        send(current_process->sys_mgr, current_thread, (uintptr_t) &current_thread->t_s);
        ((state_t *) SYSBK_OLDAREA)->a3 = (unsigned int) NULL; //la recv dal sysmgr non deve restituire alcun messaggio!
        recv(current_process->sys_mgr, NULL); //mi metto in attesa del manager

    } else if (current_process->pgm_mgr) {
        tprint("syscall_other pgm_mgr\n");

        current_thread->t_s = *((state_t *) SYSBK_OLDAREA); // salvataggio stato del processore
        //current_thread->t_s.pc -= 4;    //non serve se c'e il mgr ci deve pensare lui...

        send(current_process->pgm_mgr, current_thread, (uintptr_t) &current_thread->t_s);
        thread_enqueue(current_thread, &blockedq);
        scheduler();
    } else {
    // il thread chiamante deve essere terminato
        //tprint("syscall_other terminate_thread\n");
        terminate_thread_s(current_thread);
        scheduler();
    }

    tprint("syscall_other should not arrive here!\n");
    PANIC();
}

/***************************************************************************/

#define SYSCALL_ARG(N)  \
    (((state_t *) SYSBK_OLDAREA)->a ## N)

void syscall_h(void)
{
    if (CAUSE_EXCCODE_GET(((state_t *) SYSBK_OLDAREA)->CP15_Cause) == EXC_BREAKPOINT) {
        // tprint("\nA breakpoint occurred\n");
        LDST((state_t *) SYSBK_OLDAREA);
    }

    switch (SYSCALL_ARG(1)) {
        case SYS_ERR:
        // syscall 0 è da specifica sempre un errore
            raise_reserved_instruction();
            // FIXME: not correct; should be illegal instruction or something like that
        case SYS_SEND:
            send_kernel((struct tcb_t *) SYSCALL_ARG(2), SYSCALL_ARG(3));
        case SYS_RECV:
            recv_kernel((struct tcb_t *) SYSCALL_ARG(2), (uintptr_t *) SYSCALL_ARG(3));
        default:
            syscall_other();
    }
}
