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
#define PRINTADDR       0x1C0
#define NETADDR         0x140
#define TAPEADDR        0x0C0
#define DISKADDR        0x040
struct tcb_t* terminal0_status[8];

static inline int get_errno_s(const struct tcb_t *applicant);

static inline struct tcb_t *create_process_s(const state_t *initial_state, struct tcb_t *applicant);
static inline struct tcb_t *create_thread_s(const state_t *initial_state, struct tcb_t *process);

static inline void terminate_process_s(struct pcb_t *proc);
static inline void terminate_thread_s(struct tcb_t *thread);

static inline struct tcb_t *setpgmmgr_s(struct tcb_t *thread, struct tcb_t *applicant, int *send_back);
static inline struct tcb_t *settlbmgr_s(struct tcb_t *thread, struct tcb_t *applicant, int *send_back);
static inline struct tcb_t *setsysmgr_s(struct tcb_t *thread, struct tcb_t *applicant, int *send_back);

static inline unsigned int getcputime_s(const struct tcb_t *applicant);
static inline unsigned int wait_for_clock_s(struct tcb_t *applicant);
static inline unsigned int do_io_s(devaddr device, uintptr_t command, uintptr_t data1, uintptr_t data2,struct tcb_t* requester);
static inline void setdevice(unsigned int devno, uintptr_t command);

static inline struct pcb_t *get_processid_s(const struct tcb_t *thread);
static inline struct pcb_t *get_parentprocid_s(const struct pcb_t *proc);


struct tcb_t *SSI;

struct tcb_t *ssi_thread_init() {
    static struct tcb_t _SSI;

    _SSI.t_pcb = NULL;
    _SSI.t_status = T_STATUS_READY;
    _SSI.t_wait4sender = NULL;

    INIT_LIST_HEAD(&_SSI.t_msgq);
    INIT_LIST_HEAD(&_SSI.t_wait4me);

    return(SSI = &_SSI);
}

static inline uintptr_t req_field(uintptr_t request, int i) {
    return ((uintptr_t *) request)[i];
}

void ssi(){
    tprint("SSI started\n\n");
    while (1) {
        uintptr_t msg, reply;
        int send_back;
        struct tcb_t *applicant = msgrecv(NULL, &msg);
        tprintf("SSI request received: applicant == %p\n", applicant);

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
                if((devaddr) applicant < (devaddr) 0x00007000){//interrupt_h ci sta dicendo che un device ha completato
                        // msgsend(requester,status);
                    }
                else {} //no interrupts were found
                break;
                do_io_s(req_field(msg,1), req_field(msg,2), (uintptr_t) NULL, (uintptr_t) NULL, applicant);
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

static inline int get_errno_s(const struct tcb_t *applicant){
  return applicant->errno;
}

static inline struct tcb_t *create_process_s(const state_t *initial_state, struct tcb_t *applicant){
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

static inline struct tcb_t * create_thread_s(const state_t *initial_state, struct tcb_t *applicant){

    struct tcb_t * new_thread = thread_alloc(get_processid_s(applicant));
    if(!new_thread)
        return NULL;

    new_thread->t_s = *initial_state; //memcpy
    return new_thread;
}


static inline void __terminate_thread_s(struct tcb_t *thread);


/* Preconditions: process != NULL */
static inline void terminate_process_s(struct pcb_t *proc){
    tprint("terminate process started\n");

    // eliminiamo tutti i thread
    struct tcb_t *thread_term;
    while (thread_term = proc_firstthread(proc))
        // terminate thread changes the structure
        __terminate_thread_s(thread_term);

    // eliminiamo i figli del processo ricorsivamente
    struct pcb_t *proc_term;
    while (proc_term = proc_firstchild(proc))
        // terminate thread changes the structure
        terminate_process_s(proc_term);

    // eliminiamo il processo
    proc_delete(proc);
}

/* terminate the thread */
static inline void __terminate_thread_s(struct tcb_t *thread) {
    tprint("__terminate_thread_s started\n");
    while (!list_empty(&thread->t_msgq))
		//cancello tutti i messaggi se ce ne sono
		msg_free(msg_qhead(&thread->t_msgq));

    // sbloccare i processi in attesa di messaggi del thread da terminare
    struct tcb_t *to_resume;
    // tprintf("coda t_wait4me --> %p\n", &thread->t_wait4me);
    // tprintf("puntatori t_wait4me: next --> %p, prev --> %p\n", thread->t_wait4me.next, thread->t_wait4me.prev);
    // tprintf("la coda e' vuota? --> %d\n", list_empty(&thread->t_wait4me));

    while (to_resume = wait4thread_dequeue(&thread->t_wait4me)) {
        // tprintf("resuming thread - %p\n", to_resume);
        resume_thread(to_resume, NULL, 0);
    }
    // tprint("waiting threads resumed\n");

    int err = thread_free(thread);
    if (err == -1) {
        tprint("ERROR - msgq\n");
        HALT();
    } else if (err == -2) {
        tprint("ERROR - wait4me\n");
        HALT();
    }
    tprint("ending __terminate_thread_s\n");
}

/* terminate the thread and, if it's the last one, the process too */
static inline void terminate_thread_s(struct tcb_t *thread){
    tprint("terminate_thread_s started\n");

    if(list_is_only(&thread->t_next, &get_processid_s(thread)->p_threads))
    // se è l'unico thread del processo
        // terminiamo l'intero processo
        terminate_process_s(get_processid_s(thread));
    else
    // Se il ha fratelli
        // terminiamo unicamente questo thread
        __terminate_thread_s(thread);

    thread_count--;

    tprint("terminate_thread_s ended\n\n");
}

static inline unsigned int getcputime_s(const struct tcb_t *applicant){
    return applicant->run_time;
}

static inline unsigned int wait_for_clock_s(struct tcb_t *applicant) {
    /*code*/
}

static inline unsigned int do_io_s(devaddr device, uintptr_t command, uintptr_t data1, uintptr_t data2,struct tcb_t* requester){
    switch (device) {
        case TERM0ADDR://il device e' un terminale
            // if(/*device e' libero*/){
            //     setdevice(0,command);
            // }
            //aggiorno -> (using device)
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
static inline void setdevice(unsigned int devno, uintptr_t command){
    void * p = (void *)0x0000024c+((0x10)*devno);
    *((unsigned int *)p) = command;
}

/* *send_back è 1 se bisogna spedire una risposta al mittente, cioè il processo non è stato terminato */
static inline struct tcb_t *__setmgr(struct tcb_t *thread, struct tcb_t *applicant,
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

static inline struct tcb_t *setpgmmgr_s(struct tcb_t *thread, struct tcb_t *applicant, int *send_back) {
	return __setmgr(thread, applicant, &applicant->t_pcb->pgm_mgr, send_back);
}

static inline struct tcb_t *settlbmgr_s(struct tcb_t* thread, struct tcb_t *applicant, int *send_back) {
    return __setmgr(thread, applicant, &applicant->t_pcb->tlb_mgr, send_back);
}

static inline struct tcb_t *setsysmgr_s(struct tcb_t* thread, struct tcb_t *applicant, int *send_back) {
    return __setmgr(thread, applicant, &applicant->t_pcb->sys_mgr, send_back);
}

static inline struct pcb_t *get_processid_s(const struct tcb_t *thread){
    return thread->t_pcb;
}

static inline struct pcb_t *get_parentprocid_s(const struct pcb_t *proc){
    return proc->p_parent;
}
