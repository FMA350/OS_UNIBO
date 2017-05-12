#include <listx.h>
#include <ssi.h>
#include <uARMtypes.h>
#include <libuarm.h>
#include <syslib.h>
#include <mikabooq.h>
#include <nucleus.h>
#include <scheduler.h>
#include <syscall.h>


#define TERM0ADDR       0x24C
#define PRINTADDR       0x1c0
#define NETADDR         0x140
#define TAPEADDR        0x0C0
#define DISKADDR        0x040


static inline int get_errno_s(struct tcb_t *applicant);

static inline struct tcb_t *create_process_s(state_t *initial_state, struct tcb_t *applicant);
static inline struct tcb_t *create_thread_s(state_t *initial_state, struct tcb_t *process);

static inline void terminate_process_s(struct pcb_t *proc);
static inline void terminate_thread_s(struct tcb_t *thread);

static inline struct tcb_t *setpgmmgr_s(struct tcb_t* thread, struct tcb_t *applicant, int *send_back);
static inline struct tcb_t *settlbmgr_s(struct tcb_t* thread, struct tcb_t *applicant, int *send_back);
static inline struct tcb_t *setsysmgr_s(struct tcb_t* thread, struct tcb_t *applicant, int *send_back);

static inline unsigned int getcputime_s(struct tcb_t *applicant);
static inline unsigned int wait_for_clock_s(/*args*/);
static inline unsigned int do_io_s(devaddr device, uintptr_t command, uintptr_t data1, uintptr_t data2);

static inline struct pcb_t *get_processid_s(struct tcb_t *thread);
static inline struct pcb_t *get_parentprocid_s(struct pcb_t *proc);


struct tcb_t *SSI;

struct tcb_t *ssi_thread_init() {
    static struct tcb_t _SSI;

    _SSI.t_pcb = NULL;
    _SSI.t_status = T_STATUS_READY;
    _SSI.t_wait4sender = NULL;

    INIT_LIST_HEAD(&_SSI.t_msgq);

    return(SSI = &_SSI);
}

static inline uintptr_t req_field(uintptr_t request, int i) {
    return ((uintptr_t *) request)[i];
}

void ssi(){
    while (1) {
        uintptr_t msg, reply;
        int send_back;
        struct tcb_t *applicant = msgrecv(NULL, &msg);

        switch (req_field(msg, 0)) {
            case GET_ERRNO:
                msgsend(applicant, (uintptr_t) get_errno_s(applicant));
                break;
            case CREATE_PROCESS:
                msgsend(applicant, (uintptr_t) create_process_s((state_t *) req_field(msg, 1), applicant));
                break;
            case CREATE_THREAD:
                msgsend(applicant, (uintptr_t) create_thread_s((state_t *) req_field(msg, 1), applicant));
                break;
            case TERMINATE_PROCESS:
                terminate_process_s(get_processid_s(applicant));
                break;
            case TERMINATE_THREAD:
                terminate_thread_s(applicant);
                break;
            case SETPGMMGR:
                reply = (uintptr_t) setpgmmgr_s((struct tcb_t *) req_field(msg, 1), applicant, &send_back);
                if (send_back)
                    msgsend(applicant, reply);
                break;
            case SETTLBMGR:
                reply = (uintptr_t) settlbmgr_s((struct tcb_t *) req_field(msg, 1), applicant, &send_back);
                if (send_back)
                    msgsend(applicant, reply);
                break;
            case SETSYSMGR:
                reply = (uintptr_t) setsysmgr_s((struct tcb_t *) req_field(msg, 1), applicant, &send_back);
                if (send_back)
                    msgsend(applicant, reply);
                break;
            case GET_CPUTIME:
                msgsend(applicant, (uintptr_t) getcputime_s(applicant));
                break;

            case WAIT_FOR_CLOCK:
                /* code */
                break;
            case DO_IO:
                /* code */
                break;
            case GET_PROCESSID:
                msgsend(applicant, (uintptr_t) get_processid_s((struct tcb_t *) req_field(msg, 1)));
                break;
            case GET_PARENTPROCID:
                msgsend(applicant, (uintptr_t) get_parentprocid_s((struct pcb_t *) req_field(msg, 1)));
                break;
            case GET_MYTHREADID:
                msgsend(applicant, (uintptr_t) applicant);
                break;
            default:
            // TODO: se il messaggio è diverso dai codici noti
            //       rispondere con errore e settare errno
            break;
        }
    }
}


/***********SERVICES*****************/

static inline int get_errno_s(struct tcb_t *applicant){
  return applicant->errno;
}

static inline struct tcb_t *create_process_s(state_t *initial_state, struct tcb_t *applicant){
    struct pcb_t *new_process = proc_alloc(get_processid_s(applicant));

    if(new_process == NULL)
    return NULL;

    struct tcb_t *first_thread = thread_alloc(new_process);
    if(!first_thread){
        // throw an error, no more space availeable
        proc_delete(new_process);
        return NULL;
    }
    first_thread->t_s = *initial_state; //memcpy
    thread_enqueue(first_thread, &readyq);

    return first_thread;
}

static inline struct tcb_t * create_thread_s(state_t * initial_state, struct tcb_t *applicant){

    struct tcb_t * new_thread = thread_alloc(get_processid_s(applicant));
    if(!new_thread)
    return NULL;

    new_thread->t_s = *initial_state; //memcpy
    return new_thread;
}


static inline void __terminate_thread_s(struct tcb_t *thread);


/* Preconditions: process != NULL */
static inline void terminate_process_s(struct pcb_t *proc){

    // eliminiamo tutti i thread
    struct tcb_t *thread_iter;
    // FIXME: structure changes as we iterate. Dovrebbero funzionare, ma per sbaglio (michele)
    // si può fare con un while che estrae tutto
    list_for_each_entry(thread_iter, &proc->p_threads, t_next)
        __terminate_thread_s(thread_iter);

    // eliminiamo i figli del processo ricorsivamente
    struct pcb_t *proc_iter;
    // FIXME: structure changes as we iterate. Dovrebbero funzionare, ma per sbaglio (michele)
    list_for_each_entry(proc_iter, &proc->p_children, p_siblings)
        terminate_process_s(proc_iter);

    proc_delete(proc);
}

/* terminate the thread */
static inline void __terminate_thread_s(struct tcb_t *thread) {
    while (!list_empty(&thread->t_msgq))
		//cancello tutti i messaggi se ce ne sono
		msg_free(msg_qhead(&thread->t_msgq));

    // sbloccare i processi in attesa di messaggi del thread da terminare
    struct tcb_t *to_resume;
    // FIXME: thread_dequeue works only for list_head t_wait4me
    while (to_resume = wait4thread_dequeue(&thread->t_wait4me))
        resume_thread(to_resume, NULL, 0);


    thread_free(thread);
}

/* terminate the thread and, if it's the last one, the process too */
static inline void terminate_thread_s(struct tcb_t *thread){

    if(list_is_last(&thread->t_next, &get_processid_s(thread)->p_threads))
    // se è l'unico thread del processo
        // terminiamo anche il processo
        terminate_process_s(get_processid_s(thread));
    else
    // Se il ha fratelli
        // terminiamo unicamente questo thread
        __terminate_thread_s(thread);
}

static inline unsigned int getcputime_s(struct tcb_t *applicant){
    return applicant->run_time;
}

/* *send_back è 1 se bisogna spedire una risposta al mittente, cioè il processo non è stato terminato */
static inline struct tcb_t *
__setmgr(struct tcb_t *thread, struct tcb_t *applicant,
         struct tcb_t **mgr, int *send_back) {
    if (thread) {
        *send_back = 1;
        return NULL;
    }
	if(*mgr) {
    // se il manager è già settato
        *send_back = 0;
		terminate_process_s(get_processid_s(applicant));
		return NULL;
	} else {
    // se il manager non è mai stato settato
        *send_back = 1;
		return *mgr = thread;
    }
}

static inline unsigned int do_io_s(devaddr device, uintptr_t command, uintptr_t data1, uintptr_t data2){
    switch (device) {
        case TERM0ADDR://il device e' un terminale

            break;
        case PRINTADDR:
            break;
        case NETADDR:
            break;
        case TAPEADDR:
            break;
        case DISKADDR:
            break;
        default: return -1;
    }
}

static inline struct tcb_t *setpgmmgr_s(struct tcb_t *thread, struct tcb_t *applicant, int *send_back) {
	return __setmgr(thread, applicant, &applicant->t_pcb->pgm_mgr, send_back);
}

static inline struct tcb_t *settlbmgr_s(struct tcb_t* thread, struct tcb_t *applicant, int *send_back) {
    return __setmgr(thread, applicant, &applicant->t_pcb->tlb_mgr, send_back);
}

static inline struct tcb_t *setsysmgr_s(struct tcb_t* thread, struct tcb_t *applicant, int *send_back) {
    return __setmgr(thread, applicant, &applicant->t_pcb->sys_mgr, send_back);
}

static inline struct pcb_t *get_processid_s(struct tcb_t *thread){
    return thread->t_pcb;
}

static inline struct pcb_t *get_parentprocid_s(struct pcb_t *proc){
    return proc->p_parent;
}
