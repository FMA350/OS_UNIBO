#ifndef HANDLERS_H
#define HANDLERS_H

/* Devices and Interval Timer Interrupt handler */
void interrupt_h();

void pgmtrap_h();

void tlbtrap_h();

void syscall_h();

#define SEND_SUCCESS    0

#define SEND_FAILURE    -1

#define RECV_FAILURE    NULL

// La receive in caso di successo ritorna l'indirizzo del sender

/*
 * This function is used to resume a thread blocked while waiting for a message
 *
 * Preconditions:
 * resuming is currently in the blocked queue and it's waiting wether from a
 * specific thread or from anyone.
 * recv_rval is what the resuming thread will return from msgrecv (sender on
 * success, NULL on failure)
 *
 * Postconditions: resuming resume his execution and receves msg as message and
 * recv_rval as return value. resuming will not be outqueued on his field
 * wait4same
 */
void resume_thread(struct tcb_t *resuming, struct tcb_t *recv_rval, uintptr_t msg);
// usata in terminate_thread_s

//
unsigned int send(struct tcb_t *dest, struct tcb_t *sender, uintptr_t msg);

#endif
