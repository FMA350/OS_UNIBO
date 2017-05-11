#include <listx.h>
#include <ssi.h>
#include <uARMtypes.h>
#include <libuarm.h>
#include <syslib.h>
#include <mikabooq.h>
#include <nucleus.h>
#include <scheduler.h>


static inline int _get_errno(struct tcb_t *applicant);
static struct tcb_t * _create_process(state_t *initial_state, struct tcb_t *applicant);
static struct tcb_t * _create_thread(state_t *initial_state, struct tcb_t *process);
static void _terminate_process(struct tcb_t *applicant);
static void _terminate_thread(struct tcb_t *applicant);
static struct tcb_t * _setpgmmgr(struct tcb_t* thread, struct tcb_t *applicant);
static struct tcb_t * _settlbmgr(/*args*/);
static struct tcb_t * _setsysmgr(/*args*/);
static unsigned int _getcputime(struct tcb_t *applicant);
static unsigned int _wait_for_clock(/*args*/);
// static /*status*/ _do_io(/*args*/);
static struct pcb_t *_get_processid(struct tcb_t *thread);
static struct tcb_t *_get_mythreadid(struct tcb_t *applicant);
static struct pcb_t *_get_parentprocid(struct pcb_t *proc);

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
        uintptr_t msg;
        struct tcb_t *applicant = msgrecv(NULL, &msg);

        switch (req_field(msg, 0)) {
            case GET_ERRNO:
                msgsend(applicant, (uintptr_t) _get_errno(applicant));
                break;
            case CREATE_PROCESS:
                msgsend(applicant, (uintptr_t) _create_process(req_field(msg, 1), applicant));
                break;
            case CREATE_THREAD:
                msgsend(applicant, (uintptr_t) _create_thread(req_field(msg, 1), applicant));
                break;
            case TERMINATE_PROCESS:
                /* code */
                break;
            case TERMINATE_THREAD:
                /* code */
                break;
            case SETPGMMGR:
                /* code */
                break;
            case SETTLBMGR:
                /* code */
                break;
            case SETSYSMGR:
                /* code */
                break;
            case GET_CPUTIME:
                msgsend(applicant, (uintptr_t) _getcputime(applicant));
                break;
            case WAIT_FOR_CLOCK:
                /* code */
                break;
            case DO_IO:
                /* code */
                break;
            case GET_PROCESSID:
                msgsend(applicant, (uintptr_t) _get_processid((struct tcb_t *) req_field(msg, 1)));
                break;
            case GET_PARENTPROCID:
                msgsend(applicant, (uintptr_t) _get_parentprocid((struct tcb_t *) req_field(msg, 1)));
                break;
            case GET_MYTHREADID:
                msgsend(applicant, (uintptr_t) _get_mythreadid(applicant));
                break;
            default:
            // TODO: se il messaggio è diverso dai codici noti
            //       rispondere con errore e settare errno
                break;
        }
    }
}


/***********SERVICES*****************/

static inline int _get_errno(struct tcb_t *applicant){
  return applicant->errno;
}

static inline struct tcb_t *_create_process(state_t *initial_state, struct tcb_t *applicant){
    struct pcb_t *new_process = proc_alloc(_get_processid(applicant));

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

static inline struct tcb_t * _create_thread(state_t * initial_state, struct tcb_t *applicant){

    struct tcb_t * new_thread = thread_alloc(_get_processid(applicant));
    if(!new_thread)
        return NULL;

    new_thread->t_s = *initial_state; //memcpy
    return new_thread;
}

static void _terminate_process(struct tcb_t *processToDelete){
  if(!processToDelete) return -1;
  struct tcb_t * threadToDelete;
  while(threadToDelete = thread_qhead(&processToDelete->t_next)){
	 _terminate_thread(threadToDelete);
    }
  return proc_delete(processToDelete);
}

static void _terminate_thread(struct tcb_t *applicant){
	while (!list_empty(&applicant->t_msgq)) {
		//cancello tutti i messaggi se ce ne sono
		msg_free(msg_qhead(&applicant->t_msgq));
   	}
	//TODO:sbloccare i processi in attesa di messaggi da applicant
  
  if(thread_qhead(&applicant->t_next)==NULL){
    //only if this thread does not have any siblings
      thread_free(applicant);
      _terminate_process(applicant->t_pcb);
  }
  thread_free(applicant);
  
  /*else{ //non ho capito a cosa serva questa parte :')
    struct list_head *temp = &threadToDelete->t_pcb->p_threads;
    while(temp!=threadToDelete){
      list_next(temp);
    }
    thread_dequeue(temp);
  }*/
	
  
}

static inline unsigned int _getcputime(struct tcb_t *applicant){
    return applicant->run_time;
}

static inline struct tcb_t *_setpgmmgr(struct tcb_t *s, struct tcb_t *applicant){
	if (s!=NULL){
		if(&applicant->t_pcb->pgm_mgr != NULL) {//controllo che la chiamata sia stata fatta una sola volta
			_terminate_process(&applicant->t_pcb);
			return NULL;
		}
		else{
			applicant->t_pcb->pgm_mgr = s;
			msgsend(SSI,&s->t_s);
			return s;
		}
	}
	else return NULL;
}

static inline struct pcb_t *_get_processid(struct tcb_t *thread){
    return thread->t_pcb;
}

static inline struct tcb_t *_get_mythreadid(struct tcb_t *applicant) {
    return applicant;
}

static inline struct pcb_t *_get_parentprocid(struct pcb_t *proc){
    return proc->p_parent;
}
