#ifndef SERVICES_H
#define SERVICES_H

#include <mikabooq.h>

/* returns the errno of the applicant */
int get_errno_s(const struct tcb_t *applicant);

/*  */
struct tcb_t *create_process_s(const state_t *initial_state, struct tcb_t *applicant);
struct tcb_t *create_thread_s(const state_t *initial_state, struct tcb_t *process);

/*  */
void terminate_process_s(struct tcb_t *applicant);
void terminate_thread_s(struct tcb_t *thread);

/* Sets  */
struct tcb_t *setpgmmgr_s(struct tcb_t *new_mgr, struct tcb_t *applicant, int *send_back);
struct tcb_t *settlbmgr_s(struct tcb_t *new_mgr, struct tcb_t *applicant, int *send_back);
struct tcb_t *setsysmgr_s(struct tcb_t *new_mgr, struct tcb_t *applicant, int *send_back);

/*  Returns the time passed since the starting of the thread */
uint64_t getcputime_s(const struct tcb_t *applicant);

/*  */
void wait_for_clock_s(struct tcb_t *applicant);

/*  */
void do_io_s(devaddr device, uintptr_t command, uintptr_t data1,
            uintptr_t data2, struct tcb_t* applicant);

/*  */
struct pcb_t *get_processid_s(const struct tcb_t *thread);
struct pcb_t *get_parentprocid_s(const struct pcb_t *proc);
struct tcb_t *get_mythreadid_s(const struct tcb_t *thread);

#endif
