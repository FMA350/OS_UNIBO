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
#define MAX_REQUESTS    40

struct io_req{
    //uintptr_t val;
    devaddr device;
    struct tcb_t *thread;
};
struct io_req request[MAX_REQUESTS];

static inline int get_errno_s(const struct tcb_t *applicant);

static inline struct tcb_t *create_process_s(const state_t *initial_state, struct tcb_t *applicant);
static inline struct tcb_t *create_thread_s(const state_t *initial_state, struct tcb_t *process);

static inline void terminate_process_s(struct tcb_t *applicant);
static inline void terminate_thread_s(struct tcb_t *thread);

static inline struct tcb_t *setpgmmgr_s(struct tcb_t *thread, struct tcb_t *applicant, int *send_back);
static inline struct tcb_t *settlbmgr_s(struct tcb_t *thread, struct tcb_t *applicant, int *send_back);
static inline struct tcb_t *setsysmgr_s(struct tcb_t *thread, struct tcb_t *applicant, int *send_back);

static inline unsigned int getcputime_s(const struct tcb_t *applicant);
static inline unsigned int wait_for_clock_s(struct tcb_t *applicant);
static inline unsigned int do_io_s(uintptr_t msgg, struct tcb_t* applic);

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

    for (int i=0; i<MAX_REQUESTS; i++)
        request[i].thead = NULL;

    tprint("SSI initialized\n");

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

        tprintf("SSI request handling:\n"
                "   applicant == %p\n"
                "   request number == %d\n",
                applicant, req_field(msg, 0));

        if(applicant == NULL) {

        //interrupt_h ci sta dicendo che un device ha completato
            // FIXME - Michele: non capisco questa parte.
            //current_thread = SSI; fma: wtf?
            int i=0;
            while (request[i].requester==NULL && i<8)
                i++;
            void *p = (void *) 0x0000240;
            int status = *((int *) p);
            //tprintf("%p, %d, status:%d\n",request[i].requester,status,request[i].requester->t_status);
            msgsend(request[i].requester,status);
            request[i].val = (uintptr_t) NULL;
            request[i].requester = NULL;

            //scheduler(); fma: ssi will cycle back and block at msgrecv
            /*
            FIXME - Michele: l'SSI non dovrebbe poter chiamare lo scheduler
            direttamente. Solo l'interval timer e le syscall dovrebbero averne
            il permesso.
            */
        }
        else {
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
                terminate_process_s(applicant);
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
                do_io_s(msg, applicant);
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
            case ACK_IO:
                //in case a device completed Input/output
                io_handler(msg);
            break;
            default:
            // TODO: se il messaggio è diverso dai codici noti
            //       rispondere con errore e settare errno
            break;
        }
    }
    }
}


/***********SERVICES*****************/

static inline int get_errno_s(const struct tcb_t *applicant)
{
    return applicant->errno;
}

static inline struct tcb_t *__create_thread_s(const state_t *initial_state, struct pcb_t *proc);

static inline struct tcb_t *create_process_s(const state_t *initial_state, struct tcb_t *applicant)
{
    struct pcb_t *new_process = proc_alloc(get_processid_s(applicant));

    if(new_process == NULL) {
        return NULL;
    }

    struct tcb_t *first_thread = __create_thread_s(initial_state, new_process);

    if(!first_thread){
        // throw an error, no more space availeable
        proc_delete(new_process);
        return NULL;
    }

    thread_enqueue(first_thread, &readyq);
    return first_thread;
}

static inline struct tcb_t *__create_thread_s(const state_t *initial_state, struct pcb_t *proc)
{
    struct tcb_t *new_thread = thread_alloc(proc);
    if(!new_thread) {
        return NULL;
    }

    new_thread->t_s = *initial_state; //memcpy

    thread_count++;
    return new_thread;
}

static inline struct tcb_t * create_thread_s(const state_t *initial_state, struct tcb_t *applicant)
{
    return __create_thread_s(initial_state, get_processid_s(applicant));
}

/************************* TERMINATION *************************************/

static inline void __terminate_thread_s(struct tcb_t *thread);

/* Cleans the eventual messages sent to managers and SSI
   Preconditions: the thread is waiting

   sys_mgr e pgm_mgr sono gli unici thread di sistema oltre all'SSI
   nei confronti dei quali è possibile fare la recv (syscall_other) */
static inline void clean_sys_msg(struct tcb_t *terminating)
{
    if (terminating->t_wait4sender == SSI) {
        // msgq_get should always succeed
        msgq_get(&terminating, SSI, NULL);
    } else if (terminating->t_wait4sender == get_processid_s(terminating)->sys_mgr) {
        msgq_get(&terminating, get_processid_s(terminating)->sys_mgr, NULL);
    } else if (terminating->t_wait4sender == get_processid_s(terminating)->pgm_mgr) {
        msgq_get(&terminating, get_processid_s(terminating)->sys_mgr, NULL);
    }
}


/*
 * Terminate the process and all his progeny
 *
 * Preconditions: process != NULL, applicant is the requestor of the service
 *
 */
static inline void __terminate_process_s(struct pcb_t *proc, struct tcb_t *applicant)
{
    // TODO: eliminare dalla coda dei messaggi dell'SSI eventuali messaggi
    // provenienti dai thread dei processi figli che saranno terminati
    tprint("terminate process started\n");

    // eliminiamo tutti i thread
    struct tcb_t *thread_term;
    while (thread_term = proc_firstthread(proc)) {
        // terminate thread changes the structure
        if (thread_term != applicant && thread_term->t_status == T_STATUS_W4MSG) {
        // Se il thread non è quello che ha richiesto il servizio e sta aspettando
            clean_sys_msg(thread_term);
        }
        __terminate_thread_s(thread_term);
    }

    // eliminiamo i figli del processo ricorsivamente
    struct pcb_t *proc_term;
    while (proc_term = proc_firstchild(proc)) {
        __terminate_process_s(proc_term, NULL);
    }

    // eliminiamo il processo
    proc_delete(proc);
}

static inline void terminate_process_s(struct tcb_t *applicant)
{
    __terminate_process_s(get_processid_s(applicant), applicant);
}

/* terminate the thread. thread != NULL
   the process is also removed from the scheduling queue he's in (device queue also)*/
static inline void __terminate_thread_s(struct tcb_t *thread)
{
    tprint("__terminate_thread_s started\n");
    while (!list_empty(&thread->t_msgq))
    //cancello tutti i messaggi se ce ne sono
        msg_free(msg_qhead(&thread->t_msgq));

    // sbloccare i processi in attesa di messaggi del thread da terminare
    struct tcb_t *to_resume;
    // tprintf("coda t_wait4me --> %p\n", &thread->t_wait4me);
    // tprintf("puntatori t_wait4me: next --> %p, prev --> %p\n", thread->t_wait4me.next, thread->t_wait4me.prev);
    // tprintf("la coda e' vuota? --> %d\n", list_empty(&thread->t_wait4me));

    while (to_resume = thread_qhead(&thread->t_wait4me)) {
        // tprintf("resuming thread - %p\n", to_resume);
        resume_thread(to_resume, NULL, 0);
    }
    // tprint("waiting threads resumed\n");

    int err = thread_free(thread);
    // if (err == -1) {
    //     tprint("ERROR - msgq\n");
    //     HALT();
    // } else if (err == -2) {
    //     tprint("ERROR - wait4me\n");
    //     HALT();
    // }
    // tprint("ending __terminate_thread_s\n");

    thread_count--;
}

/* terminate the thread and, if it's the last one, the process too */
static inline void terminate_thread_s(struct tcb_t *thread)
{
    tprint("terminate_thread_s started\n");

    if(list_is_only(&thread->t_next, &get_processid_s(thread)->p_threads)) {
    // se è l'unico thread del processo
        // terminiamo l'intero processo
        terminate_process_s(thread);
    } else {
    // Se il ha fratelli
        // terminiamo unicamente questo thread
        __terminate_thread_s(thread);
    }

    tprint("terminate_thread_s ended\n\n");
}

/****************************************************************************/

static inline unsigned int getcputime_s(const struct tcb_t *applicant)
{
    return applicant->run_time;
}

static inline unsigned int wait_for_clock_s(struct tcb_t *applicant)
{
    /*code*/
}

static inline void setdevice(unsigned int devno, uintptr_t command){
    //tprint("setting transmitChar command\n");

    // FIXME - Michele: Ma cosa porcoddio succede qui?
    void *p = (void *) 0x0000024c + ((0x10)*devno);
    *((unsigned int *)p) = command;

    //tprint("\n..........completed transmitChar command\n");
}

static inline unsigned int do_io_s(uintptr_t msgg, struct tcb_t* applic){
    //tprint("    do_io_s started\n");
/*    switch (req_field(msgg,1)) {
        case TERM0ADDR:   //il device e' un terminale*/
        // int empty = 1;
        // int i=0; //fma: i has to be initialized!

        // the thread gets soft blocked
        // soft_block_count++;
        //
        // while (request[i].requester==NULL && i<8)
        //     i++;
        //
        // if (request[i].requester!=NULL)
        //     empty = 0;
        //
        // if(empty){
        //     setdevice(0,req_field(msgg,2));
        //     request[0].val = msgg;
        //     request[0].requester = applic;
        // }
        // //aggiorno -> (using device)
        // else {
        //     i=0;
        //     while(request[i].requester!=NULL && i<8)
        //         i++; //cerco il primo buco libero per salvare il messaggio
        //
        //     if (i==8)
        //         return -1; //se non ci sono piu spazi per salvare...
        //     else {
        //         request[i].val = msgg;
        //         request[i].requester = applic;
        //     }
        // }
        /*break;

        case PRINTADDR:
        break;
        case NETADDR:
        break;
        case TAPEADDR:
        break;
        case DISKADDR:
        break;
        default: return -1;
    }*/
    // tprint("    do_io_s finished\n");
    //*****fmacode*****
    //fma: the ssi doesn't have to worry about having more requests
    //on the same device at the same time. The Device Manager will
    //take care about it.
    soft_block_count++; //TODO: shouldn't this be moved to the msgsend DO_IO of the thread.
    devaddr device = req_field(msgg,2);
    uintptr_t command = req_field(msgg,3);
    uintptr_t data1 = req_field(msgg,4);
    uintptr_t data2 = req_field(msgg,5);


    *device+0x40 = data1;
    *device+0x60 = data2;
    *device+0x20 = command; //writing the command at this location
                            //activates the device.
    //let's save this request in a queue so we can find it later
    int i = 0;
    while (request[i].thread==NULL && i<MAX_REQUESTS){
        //look for an empty spot
        //TODO: optimization! let's make it work like a bitmap!
        i++;
    }
    request[i].thread = applic;
    request[i].device = device; //not needed once we will have the bitmap!
}

io_handler(uintptr_t msgg){
    soft_block_count--;
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
            terminate_process_s(applicant);
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
