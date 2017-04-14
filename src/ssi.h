#include <mikabooq.h>
#include <listx.h>
#include <arch.h>
#include <uARMconst.h>
#include <sys/types.h>
#include <uARMtypes.h>
#include <interrupts.h>
#include <const.h>

void SSI();

int errorNumber;

/*SERVICES*/
int GET_ERRNO();

struct pcb_t * CREATE_PROCESS(state_t initial_state);

struct tcb_t * CREATE_THREAD(state_t initial_state, struct pcb_t * process);

int TERMINATE_PROCESS(struct pcb_t *processToDelete);

int TERMINATE_THREAD(struct tcb_t * threadToDelete);
