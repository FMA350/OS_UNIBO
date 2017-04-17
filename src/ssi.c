/*
   _____            __                    _____                 _              ____      __            ____
  / ___/__  _______/ /____  ____ ___     / ___/___  ______   __(_)_______     /  _/___  / /____  _____/ __/___ _________
  \__ \/ / / / ___/ __/ _ \/ __ `__ \    \__ \/ _ \/ ___/ | / / / ___/ _ \    / // __ \/ __/ _ \/ ___/ /_/ __ `/ ___/ _ \
 ___/ / /_/ (__  ) /_/  __/ / / / / /   ___/ /  __/ /   | |/ / / /__/  __/  _/ // / / / /_/  __/ /  / __/ /_/ / /__/  __/
/____/\__, /____/\__/\___/_/ /_/ /_/   /____/\___/_/    |___/_/\___/\___/  /___/_/ /_/\__/\___/_/  /_/  \__,_/\___/\___/
     /____/
*/

#include <listx.h>
#include <ssi.h>
#include <uARMtypes.h>
#include <libuarm.h>
#include <syslib.h>
#include <nucleus.h>


static inline void DISPATCH(unsigned int MSG) {
    switch (MSG) {
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
        case GETCPUTIME:
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
        case GET_THREAD:
            /* code */
            break;
        default:
        // TODO: se il messaggio è diverso dai codici noti
    }
}

void SSI(){
    tprint("SSI started\n");

    while (1) {

        unsigned int msg;
        struct tcb_t *applicant = msgrecv(NULL, &msg);

        DISPATCH(msg);

        // send response back
        // TODO: send payload back
        if (msgsend(applicant, /* payload */) == -1) {
        // if msgsend fails
        // TODO: find a better solution
            PANIC();
        }

    }
}


/***********SERVICES*****************/

int GET_ERRNO(){
  return errorNumber;
}

/* FIXME: passing a pointer as an argument is more efficient */
struct pcb_t * CREATE_PROCESS(state_t initial_state){
  struct pcb_t * new_process = proc_alloc(NULL);    // mnalli: proc_alloc(NULL) ?
  if(!new_process){
    //TODO: throw an error, no more space availeable
    return NULL; //si, in teoria l'oggetto new_process e' gia' null...
  }
  struct tcb_t * first_thread = thread_alloc(new_process);
  if(!first_thread){
    //TODO: throw an error, no more space availeable
  }
  first_thread->t_s = initial_state; //memcpy
  thread_enqueue(first_thread, &new_process->p_threads);
  //all good
  return new_process;
}

struct tcb_t * CREATE_THREAD(state_t initial_state, struct pcb_t * process){
  struct tcb_t * new_thread = thread_alloc(process);
  if(!new_thread){
    //TODO: throw an error, no more space availeable
    return NULL;
  }
  new_thread->t_s = initial_state; //memcpy
  //all good
  return new_thread;
}

int TERMINATE_PROCESS(struct pcb_t *processToDelete){
  if(!processToDelete) return -1;
  struct tcb_t * threadToDelete;
  while(threadToDelete = thread_dequeue(&processToDelete->p_threads)){
    if(thread_free(threadToDelete)==-1){
      //could not close a specific thread since
      //some messages are still in the queue.
      //FIXME: should it stop like now or should it keep going?
      return -1;
    }
  }
  return proc_delete(processToDelete);
}

int TERMINATE_THREAD(struct tcb_t *threadToDelete){
  if(!threadToDelete) return -1;

  if(!(thread_qhead(&threadToDelete->t_next))){
    //only if this thread does not have any siblings
      return TERMINATE_PROCESS(threadToDelete->t_pcb);
  }
  else{
    struct list_head *temp = &threadToDelete->t_pcb->p_threads;
    while(temp!=threadToDelete){
      list_next(temp);
    }
    thread_dequeue(temp);
  }
  return thread_free(threadToDelete);
}

/*
stuff
*/

unsigned int GETCPUTIME(struct tcb_t * thread){

}

struct pcb_t *GET_PROCESSID(struct tcb_t * thread){
  return thread->t_pcb; //TODO: What do they really want? Documentation isn't clear.
}
