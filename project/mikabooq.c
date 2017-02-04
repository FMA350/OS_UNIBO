#include "mikabooq.h"
#include "const.h"
#include "listx.h"

/* Space allocated for processes */
struct pcb_t process[MAXPROC];

/* Space alloccated for threads */
struct tcb_t thread[MAXTHREAD];
struct list_head thread_h;

#if 0
/*  Space allocated for messages */
struct msg_t message[MAXMSG];
struct list_head message_h;
#endif


//mnalli

struct pcb_t *proc_init(void){

    /* process[0] is the root process */
    process[0].p_parent = NULL;
    INIT_LIST_HEAD(&process[0].p_threads);
    INIT_LIST_HEAD(&process[0].p_children);
    INIT_LIST_HEAD(&process[0].p_siblings);

    /* We do not initialize any field here */
    for (size_t i = 1; i < MAXPROC; i++)
        /* the sentinel of the free list is process[0].p_siblings */
        list_add_tail(&process[i].p_siblings, &(process[0].p_siblings));

    return process;
}

struct pcb_t *proc_alloc(struct pcb_t *p_parent){
    /* We extract the space from the free list */
    struct list_head *new_space = list_next(&(process[0].p_siblings));
    if (new_space == NULL) {
        /* Out of memory */;
    }
    /* Deleting the element from the free list */
    list_del(new_space);

    struct pcb_t *new_proc = container_of(new_space, struct pcb_t, p_siblings);

    new_proc->p_parent = p_parent;
    INIT_LIST_HEAD(&new_proc->p_threads);

    /* first_child is the first child of p_parent if p_parent has a child.
       It is NULL or one of the ancestrors if not */
    struct pcb_t *first_child = proc_firstchild(p_parent);

    if (first_child) {
        /* if p_parent has at least one child (first_child is the elder child) */
        list_add_tail(&(new_proc->p_siblings), &(first_child->p_siblings));
        INIT_LIST_HEAD(&new_proc->p_children);
    } else {
        /* if p_parent doesn't have any child */
        list_add(&(new_proc->p_children), &(p_parent->p_children));
        INIT_LIST_HEAD(&new_proc->p_siblings);
    }

    return new_proc;
}

/* delete a process (properly updating the process tree links) */
/* this function must fail if the process has threads or children. */
/* return value: 0 in case of success, -1 otherwise */
/*
 * DOESN'T WORK WITH ROOT PROCESS (SEGMENTATION FAULT)
 * IS THAT A PROBLEM???????????????????????
 */

int proc_delete(struct pcb_t *oldproc){
    if (oldproc->p_parent == NULL) {
        /* Trying to delete root or a non-allocated process */
        return -1;
    }

    if (proc_firstchild(oldproc) == NULL && proc_firstthread(oldproc) == NULL) {
        /* the process can be deleted */

        /* Parent of oldproc */
        struct pcb_t *oldproc_parent = oldproc->p_parent;

        oldproc->p_parent = NULL;

        if (proc_firstchild(oldproc_parent) == oldproc) {
            /* oldproc is the first child */
            list_del(&(oldproc->p_children));
            struct line_head *next_sibling = list_next(&(oldproc->p_siblings));

            if (next_sibling) {
                /* if oldproc isn't the only child */
                list_del(&(oldproc->p_siblings));
                list_add(&(container_of(next_sibling, struct pcb_t, p_siblings)->p_children), &(oldproc_parent->p_children));
            }

            /* add oldproc tho the free list */
            list_add_tail(&(oldproc->p_siblings), &(process[0].p_siblings));

        } else {
            /* oldproc isn't the first child */
            list_del(&(oldproc->p_siblings));
            /* add oldproc tho the free list */
            list_add_tail(&(oldproc->p_siblings), &(process[0].p_siblings));
        }

        return 0;
    } else
        /* the process has got children or threads */
        return -1;
}

/* return the pointer to the first child (NULL if the process has no children) */
struct pcb_t *proc_firstchild(struct pcb_t *proc) {
    struct list_head *first_child = list_next(&(proc->p_children));
    if (first_child == NULL || proc != (container_of(first_child, struct pcb_t, p_children)->p_parent))
        /* if p_parent doesn't have any child */
        return NULL;
    else
        /* if p_parent has at least one child (first_child is the elder child) */
        return container_of(first_child, struct pcb_t, p_children);
}

struct tcb_t *proc_firstthread(struct pcb_t *proc){
    struct list_head *first_thread = list_next(&(proc->p_threads));

    if (first_thread)
        /* the process has at least one thread */
        return container_of(first_thread, struct tcb_t, t_next);
    else
        return NULL;
}


/****************************************** THREAD ALLOCATION ****************/


//FMA350
void thread_init(void){
  INIT_LIST_HEAD(&thread_h);

  for (size_t i = 0; i < MAXTHREAD; i++){
      thread[i].t_pcb = NULL;
      thread[i].t_status = T_STATUS_NONE;
      thread[i].t_wait4sender = NULL;
      list_add_tail(&thread[i].t_next, &thread_h);
      INIT_LIST_HEAD(&thread[i].t_sched);
      INIT_LIST_HEAD(&thread[i].t_msgq);
    }
}

struct tcb_t *thread_alloc(struct pcb_t *process){
    struct list_head *new_head = list_next(&thread_h);

    if(process == NULL || new_head == NULL) {
        /* ERROR! the given process pointer is NULL
           or the free list is empty */
        return NULL;
    }

    struct tcb_t *new_thread = container_of(new_head, struct tcb_t, t_next);
    //initializing the new thread
    /* removes the thread from the free list */
    list_del(new_head);
    new_thread->t_pcb = process;
    /*adds the thread to the control thread list of the process*/
    list_add_tail(new_head, &process->p_threads);
    new_thread->t_status = T_STATUS_READY; //ready to be scheduled
    return new_thread;
}

int thread_free(struct tcb_t *oldthread){
  //check that no messages are left in the queue.
  if(!list_empty(&oldthread->t_msgq)){
    return -1; //there are messsages left in the queue.
  }
  //remove the thread from the process queue.
  list_del(&oldthread->t_next);
  oldthread->t_pcb = NULL;
  oldthread->t_status = T_STATUS_NONE;
  oldthread->t_wait4sender = NULL;

  /* Non necessario, verrà inserito nella lista libera */
  //INIT_LIST_HEAD(&oldthread->t_next);
  /* Va rimosso dalla coda dello scheduler??? */
  //INIT_LIST_HEAD(&oldthread->t_sched);

  //t_msgq is already empty.
  /*adding oldthread to the free list*/
  list_add_tail(&oldthread->t_next, &(thread[0].t_next));
  return 0;
}

/*************************** THREAD QUEUE ************************/
//fma350


/* add a tcb to the scheduling queue */
inline void thread_enqueue(struct tcb_t *new, struct list_head *queue){
  list_add_tail(&new->t_next, queue);
}

/* return the head element of a scheduling queue.
	 (this function does not dequeues the element)
	 return NULL if the list is empty */
struct tcb_t *thread_qhead(struct list_head *queue){
    struct list_head *new_head = list_next(queue);
    if(new_head == NULL)
      return NULL;
    else
      /* t_next ---> t_sched */
      return container_of(new_head, struct tcb_t, t_sched);
}


/* get the first element of a scheduling queue.
	 return NULL if the list is empty */
struct tcb_t *thread_dequeue(struct list_head *queue){
  struct list_head *new_head = list_next(queue);
  if(new_head == NULL)
    return NULL;
  else{
    list_del(new_head);
    /* t_next ---> t_sched */
    return container_of(new_head, struct tcb_t, t_sched);
  }

}

/*************************** MSG QUEUE ************************/
//fma350

void msgq_init(void){
  INIT_LIST_HEAD(&message_h);
  for(int i = 0; i < MAXMSG; i++){
    list_add(&message[i].m_next, &message_h);
    message[i].m_sender = NULL;
    /* m_value is not exactly a pointer */
    message[i].m_value  = NULL;
  }
}

int msgq_add(struct tcb_t *sender, struct tcb_t *destination, uintptr_t value){
    struct list_head *new_space = list_empty(&message_h);
    if(new_space == NULL ||sender == NULL || destination == NULL)
        //free list is empty or wrong argument passed
        return -1;
    else {
        /* deleting the element from the free list */
        list_del(new_space);
        /* obtaining a pointer to the new message */
        struct msg_t *new_msg = container_of(new_space, struct msg_t, m_next);
        /* setting the fields */
        new_msg->m_sender = sender;
        new_msg->m_value = value;
        /* placing the message in the message list of destination */
        list_add_tail(new_space, &destination->t_msgq);

        return 0;
    }
}

int msgq_get(struct tcb_t **sender, struct tcb_t *destination, uintptr_t *value){
    if(sender == NULL){
        /* restituisce il primo messaggio in coda qualsiasi ne sia il mittente
         * L’indirizzo del TCB del mittente viene memorizzato in *sender */

         //concatenatore del primo messaggio in coda
         struct list_head *msg_conc = list_next(&destination->t_msgq);
         if (msg_conc == NULL)
            /* empty queue */
            return -1;
         else {
             /* removing the message from the list */
             list_del(msg_conc);
             /* extracting the vaue */
             *value = container_of(msg_conc, struct msg_t, m_next)->m_value;
             /* adding the element to the free list */
             list_add_tail(msg_conc, message_h);
             return 0;
         }

    }
    /* sender != NULL && *sender == NULL */
    else if (*sender == NULL){
        /* restituisce il primo messaggio in coda qualsiasi ne sia il mittente
         * L’indirizzo del TCB del mittente viene memorizzato in *sender */

         //concatenatore del primo messaggio in coda
         struct list_head *msg_conc = list_next(&destination->t_msgq);
         if (msg_conc == NULL)
            /* empty queue */
            return -1;
        else {
            struct msg_t *msg = container_of(msg_conc, struct msg_t, m_next)
            /* removing the message from the list */
            list_del(msg_conc);
            /* extracting value and sender */
            *value = msg->m_value;
            *sender = msg->m_sender;
            /* adding the element to the free list */
            list_add_tail(msg_conc, message_h);

            return 0;
        }
    }
    /* sender != NULL && *sender != NULL */
    else {
        /* restituisce il primo messaggio in coda che ha *sender come mittente */
        struct msg_t *pos;
        list_for_each_entry(pos, &destination->t_msgq, member){
            if(pos->m_sender == *sender){
                /* removing the message from the list */
                list_del(&pos->m_next);
                /* extracting the value */
                *value = pos->m_value;
                /* adding the element to the free list */
                list_add_tail(&pos->m_next, message_h);
                return 0;
            }
        }
        /* nessun massaggio da parte di sender è stato trovato */
        return -1;
    }
}
