
void SSI(){
    tprint("SSI started\n");
    //get all messages(requests from other threads)
    //for(till there are still messages in the queue){
      //do service request
      //send message back.}
    //prgtrap(back to the scheduler)
}

errorNumber = 0;

/***********SERVICES*****************/

int GET_ERRNO(){
  return errorNumber;
}

struct pcb_t * CREATE_PROCESS(state_t initial_state){
  pcb_t * new_process = proc_alloc();
  if(!new_process){
    //TODO: throw an error, no more space availeable
    return NULL; //si, in teoria l'oggetto new_process e' gia' null...
  }
  tcb_t * first_thread = thread_alloc();
  if(!first_thread){
    //TODO: throw an error, no more space availeable
  }
  first_thread->t_s = initial_state; //memcpy
  thread_enqueue(first_thread, new_process->p_threads);
  //all good
  return new_process;
}

struct tcb_t * CREATE_THREAD(state_t initial_state){
  tcb_t * new_thread = thread_alloc();
  if(!new_thread){
    //TODO: throw an error, no more space availeable
    return NULL;
  }
  new_thread->t_s = initial_state; //memcpy
  //all good
  return new_thread;
}

int TERMINATE_PROCESS(pcb_t *processToDelete){
  tcb_t * threadToDelete;
  while(threadToDelete = proc_firstthread(processToDelete)){
    if(thread_free(threadToDelete)==-1){
      //could not close a specific thread since
      //some messages are still in the queue.
      //FIXME: should it stop like now or should it keep going?
      return -1;
    }
  }
  if(proc_delete(pcb_t)==-1){
    return -1;
    //could not close for some reasons (has children maybe?)
  }
  return 0;
}

int TERMINATE_THREAD(tcb_t * threadToDelete){
  if(thread_free(threadToDelete)==-1){
    //error, there are still messages in the queue.
    return -1;
  }
  if(!threadToDelete->t_next){
    //only if this thread does not have any siblings
    if(proc_delete(threadToDelete->t_pcb)==-1){
      return -1
    }
  }
  else return 0;
}
