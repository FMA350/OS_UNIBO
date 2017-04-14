void SSI();

int errorNumber;

/*SERVICES*/
int GET_ERRNO();

struct pcb_t * CREATE_PROCESS(state_t initial_state);

struct tcb_t * CREATE_THREAD(state_t initial_state);

int TERMINATE_PROCESS(pcb_t *processToDelete);

int TERMINATE_THREAD(tcb_t * threadToDelete);
