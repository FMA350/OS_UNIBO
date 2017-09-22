#include <mikabooq.h>
#include "accounting.h"
#include <scheduler.h>

/*
 * In questa funzione dobbiamo distinguere due casi di possibile esecuzione:
 * 1 - Il thread richiedente invia il messaggio all'SSI e si blocca facendo
 *     la msgrecv. In questo caso il thread si trova nella lista di thread
 *     che aspettano un messaggio dall'SSI ed il suo stato sarà quindi T_STATUS_W4MSG.
 * 2 - Il thread richiedente invia il messaggio all'SSI e prima che possa fare
 *     la msgrecv viene prehempted da un interrupt dell'Interval Timer. In questo
 *     caso il thread si trova nella lista dei processi schedulati ed il suo stato
 *     sarà T_STATUS_READY.
 *
 * In ogni caso prima di sbloccare il thread è necessario rimettere le strutture
 * di dati nello stato precedente. La funzione che si occupa di questo è update_clock
 * del file accounting.c.
 */

void wait_for_clock_s(struct tcb_t *applicant)
{
    // rimosso dalla lista di scheduling in cui si trovava
    thread_outqueue(applicant);
    // il processo si blocca in attesa di un device (lo pseudoclock)
    soft_block_count++;
    // inserito nella lista di processi in attesa
    thread_enqueue(applicant, &t_wait4clock);
}
