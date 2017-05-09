#include <listx.h>
#include <ssi.h>
#include <uARMtypes.h>
#include <libuarm.h>
#include <syslib.h>
#include <mikabooq.h>
#include <nucleus.h>


static int _get_errno();
static struct pcb_t *_create_process(state_t *initial_state);
static struct tcb_t *_create_thread(state_t *initial_state, struct pcb_t *process);
static int _terminate_process(struct pcb_t *processtodelete);
static int _terminate_thread(struct tcb_t *threadtodelete);
static struct tcb_t * _setpgmmgr(struct tcb_t* thread);
static struct tcb_t * _settlbmgr(/*args*/);
static struct tcb_t * _setsysmgr(/*args*/);
static unsigned int _getcputime(struct tcb_t * thread);
static unsigned int _wait_for_clock(/*args*/);
// static /*status*/ _do_io(/*args*/);
static struct pcb_t *_get_processid(struct tcb_t * thread);
static struct pcb_t *_get_mythreadid(struct tcb_t *mythread);
static struct pcb_t *_get_parentprocid(struct pcb_t*thread);

static struct tcb_t *applicant;

struct tcb_t *SSI;

struct tcb_t *ssi_thread_init() {
    static struct tcb_t _SSI;

    _SSI.t_pcb = NULL;
    _SSI.t_status = T_STATUS_READY;
    _SSI.t_wait4sender = NULL;

    INIT_LIST_HEAD(&_SSI.t_next);
    INIT_LIST_HEAD(&_SSI.t_sched);
    INIT_LIST_HEAD(&_SSI.t_msgq);

    return(SSI = &_SSI);
}


static inline uintptr_t DISPATCH(uintptr_t msg) {


    switch (((uintptr_t *) msg)[0]) {
        case GET_ERRNO:
            /* code */
            break;
        case CREATE_PROCESS:
            /* code */
            break;
        case CREATE_THREAD:
            /* code */
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
            /* code */
            break;
        case WAIT_FOR_CLOCK:
            /* code */
            break;
        case DO_IO:
            /* code */
            break;
        case GET_PROCESSID:
            /* code */
            break;
        case GET_PARENTPROCID:
            /* code */
            break;
        case GET_MYTHREADID:
            /* code */
            break;
        default:
        // TODO: se il messaggio è diverso dai codici noti
            break;
    }

}


void ssi(){
    while (1) {
        uintptr_t msg, reply;
        //struct tcb_t *
        applicant = msgrecv(NULL, &msg);

        reply = DISPATCH(msg);

        // send response back
        if (msgsend(applicant, reply) == -1) {
        // if msgsend fails
            // TODO: find a better solution
            PANIC();
        }
    }
}


/***********SERVICES*****************/



static int _get_errno(){
  return errorNumber;
}

/* FIXME: passing a pointer as an argument is more efficient */
static struct pcb_t * _create_process(state_t *initial_state){
  struct pcb_t * new_process = proc_alloc(NULL);    // mnalli: proc_alloc(NULL) ?
  if(!new_process){
    //TODO: throw an error, no more space availeable
    return NULL; //si, in teoria l'oggetto new_process e' gia' null...
  }
  struct tcb_t * first_thread = thread_alloc(new_process);
  if(!first_thread){
    //TODO: throw an error, no more space availeable
  }
  first_thread->t_s = *initial_state; //memcpy
  thread_enqueue(first_thread, &new_process->p_threads);
  //all good
  return new_process;
}

static struct tcb_t * _create_thread(state_t * initial_state, struct pcb_t * process){
  struct tcb_t * new_thread = thread_alloc(process);
  if(!new_thread){
    //TODO: throw an error, no more space availeable
    return NULL;
  }
  new_thread->t_s = *initial_state; //memcpy
  //all good
  return new_thread;
}

static int _terminate_process(struct pcb_t *processToDelete){
  if(!processToDelete) return -1;
  struct tcb_t * threadToDelete;
  while(threadToDelete = thread_qhead(&processToDelete->p_threads)){
	while (!list_empty(&threadToDelete->t_msgq)) {
		//cancello tutti i messaggi se ce ne sono
		msg_free(msg_qhead(&threadToDelete->t_msgq));
    	}
	thread_free(threadToDelete);

      //could not close a specific thread since
      //some messages are still in the queue.
      //FIXME: should it stop like now or should it keep going?
	//direi che cancelliamo i messaggi i messaggi e via

    }

  
  return proc_delete(processToDelete);
}

static int _terminate_thread(struct tcb_t *threadToDelete){
  if(!threadToDelete) return -1;

  if(thread_qhead(&threadToDelete->t_next)==NULL){
    //only if this thread does not have any siblings
      return _terminate_process(threadToDelete->t_pcb);
  }
  /*else{ //non ho capito a cosa serva questa parte :')
    struct list_head *temp = &threadToDelete->t_pcb->p_threads;
    while(temp!=threadToDelete){
      list_next(temp);
    }
    thread_dequeue(temp);
  }*/

  return thread_free(threadToDelete);
}

/*
stuff
*/

static unsigned int _getcputime(struct tcb_t * thread){
    //FIXME
    return 0;
}

static struct tcb_t *_setpgmmgr(struct tcb_t *s){
    //come controllo che la chiamata sia stata fatta una sola volta?

    msgsend(SSI,&s->t_s);
}



static struct pcb_t *_get_processid(struct tcb_t * s){
//>>>>>>> e3800a8c01ad8101e0ad16d5bdffcb70a3a71dab
  return s->t_pcb; //TODO: What do they really want? Documentation isn't clear.
  msgsend(SSI,&s->t_s);
}

