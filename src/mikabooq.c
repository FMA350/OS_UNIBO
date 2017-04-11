#include <mikabooq.h>
#include <const.h>

/* Space allocated for processes */
static struct pcb_t process[MAXPROC];

/* Space alloccated for threads */
static struct tcb_t thread[MAXTHREAD];
static LIST_HEAD(thread_h);

/*  Space allocated for messages */
static struct msg_t message[MAXMSG];
static LIST_HEAD(message_h);



struct pcb_t *proc_init(void) {

    /* process[0] is the root process */
    process[0].p_parent = NULL;
    INIT_LIST_HEAD(&process[0].p_threads);
    INIT_LIST_HEAD(&process[0].p_children);
    INIT_LIST_HEAD(&process[0].p_siblings);

    size_t i;
    for (i = 1; i < MAXPROC; i++) {
        /* the sentinel of the free list is process[0].p_siblings */
        list_add_tail(&process[i].p_siblings, &(process[0].p_siblings));
        process[i].p_parent = NULL;
        INIT_LIST_HEAD(&process[i].p_threads);
        INIT_LIST_HEAD(&process[i].p_children);
    }
    return process;
}

struct pcb_t *proc_alloc(struct pcb_t *p_parent) {
    /* We extract the space from the free list */
    struct list_head *new_space = list_next(&(process[0].p_siblings));
    if (new_space == NULL || p_parent == NULL)
        /* the free list is empty or no parent is specified */
        return NULL;

    /* Deleting the element from the free list */
    list_del(new_space);
    struct pcb_t *new_proc = container_of(new_space, struct pcb_t, p_siblings);
    new_proc->p_parent = p_parent;
    list_add_tail(new_space, &(p_parent->p_children));

    return new_proc;
}

/* delete a process (properly updating the process tree links) */
/* this function must fail if the process has threads or children. */
/* return value: 0 in case of success, -1 otherwise */

int proc_delete(struct pcb_t *oldproc){
    if (oldproc->p_parent == NULL ||
        !list_empty(&oldproc->p_children) ||
        !list_empty(&oldproc->p_threads))
        // Trying to delete root or a non-allocated process
        // Trying to delete a process with children
        // Trying to delete a process with threads
        return -1;
    else {
        // the process can be deleted

        oldproc->p_parent = NULL;
        list_del(&oldproc->p_siblings);
        list_add_tail(&oldproc->p_siblings, &(process[0].p_siblings));

        return 0;
    }
}

/* return the pointer to the first child (NULL if the process has no children) */
inline struct pcb_t *proc_firstchild(struct pcb_t *proc) {

    struct list_head *first_child = list_next(&proc->p_children);

    if (first_child)
        //the process has a child
        //children are linked with p_sibling field
        return container_of(first_child, struct pcb_t, p_siblings);
    else
        //if p_parent doesn't have any child
        return NULL;
}

inline struct tcb_t *proc_firstthread(struct pcb_t *proc){
    struct list_head *first_thread = list_next(&proc->p_threads);

    if (first_thread)
        // the process has at least one thread
        return container_of(first_thread, struct tcb_t, t_next);
    else
        return NULL;
}


/****************************************** THREAD ALLOCATION ****************/


void thread_init(void) {
    size_t i;
    for (i = 0; i < MAXTHREAD; i++) {
        thread[i].t_pcb = NULL;
        thread[i].t_status = T_STATUS_NONE;
        thread[i].t_wait4sender = NULL;
        list_add_tail(&thread[i].t_next, &thread_h); //collego i vari elementi della lista libera
        INIT_LIST_HEAD(&thread[i].t_sched);
        INIT_LIST_HEAD(&thread[i].t_msgq);
    }
}

struct tcb_t *thread_alloc(struct pcb_t *process) {
    struct list_head *new_head = list_next(&thread_h);

    if(process == NULL || new_head == NULL)
        /* ERROR! the given process pointer is NULL
           or the free list is empty */
        return NULL;

    struct tcb_t *new_thread = container_of(new_head, struct tcb_t, t_next);

    list_del(new_head); 							/* removes the thread from the free list */
    new_thread->t_pcb = process;
    list_add_tail(new_head, &process->p_threads);   /*adds the thread to the control thread list of the process*/
    new_thread->t_status = T_STATUS_READY; 			//ready to be scheduled
    return new_thread;
}

int thread_free(struct tcb_t *oldthread) {
    //check that no messages are left in the queue.
    if(!list_empty(&oldthread->t_msgq)) {
        return -1; //there are messsages left in the queue.
    }
    //remove the thread from the process queue.
    list_del(&oldthread->t_next);
    oldthread->t_pcb = NULL;
    oldthread->t_status = T_STATUS_NONE;
    oldthread->t_wait4sender = NULL;

    //t_msgq is already empty.
    /*adding oldthread to the free list*/
    list_add_tail(&oldthread->t_next, &(thread_h));
    return 0;
}

/*************************** THREAD QUEUE ************************/


/* add a tcb to the scheduling queue */
inline void thread_enqueue(struct tcb_t *new, struct list_head *queue) { //chopperEdit
    list_add_tail(&new->t_sched, queue);
}

/* return the head element of a scheduling queue.
	 (this function does not dequeues the element)
	 return NULL if the list is empty */
struct tcb_t *thread_qhead(struct list_head *queue) { //chopperEdit
    struct list_head *new_head = list_next(queue);
    if(new_head == NULL)
        return NULL;
    else
        /* t_next ---> t_sched */
        return container_of(new_head, struct tcb_t, t_sched);
}


/* get the first element of a scheduling queue.
	 return NULL if the list is empty */
struct tcb_t *thread_dequeue(struct list_head *queue) {
    struct list_head *new_head = list_next(queue);
    if(new_head == NULL)
        return NULL;
    else {
        list_del(new_head);
        return container_of(new_head, struct tcb_t, t_sched);
    }
}

/*************************** MSG QUEUE ************************/

void msgq_init(void) {
    size_t i;
    for(i = 0; i < MAXMSG; i++) {
        list_add(&message[i].m_next, &message_h);
        message[i].m_sender = NULL;
        //message[i].m_value  = 0;
    }
}

int msgq_add(struct tcb_t *sender, struct tcb_t *destination, uintptr_t value) {
    /* extracting space from free list */
    struct list_head *new_space = list_next(&message_h);
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

int msgq_get(struct tcb_t **sender, struct tcb_t *destination, uintptr_t *value) {
    if(sender == NULL) {
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
            list_add_tail(msg_conc, &message_h);
            return 0;
        }

    }
    /* sender != NULL && *sender == NULL */
    else if (*sender == NULL) {
        /* restituisce il primo messaggio in coda qualsiasi ne sia il mittente
         * L’indirizzo del TCB del mittente viene memorizzato in *sender */

        //concatenatore del primo messaggio in coda
        struct list_head *msg_conc = list_next(&destination->t_msgq);
        if (msg_conc == NULL)
            /* empty queue */
            return -1;
        else {
            struct msg_t *msg = container_of(msg_conc, struct msg_t, m_next);
            /* removing the message from the list */
            list_del(msg_conc);
            /* extracting value and sender */
            *value = msg->m_value;
            *sender = msg->m_sender;
            /* adding the element to the free list */
            list_add_tail(msg_conc, &message_h);

            return 0;
        }
    }
    /* sender != NULL && *sender != NULL */
    else {
        /* restituisce il primo messaggio in coda che ha *sender come mittente */
        struct msg_t *pos;
        list_for_each_entry(pos, &destination->t_msgq, m_next) {
            if(pos->m_sender == *sender) {
                /* removing the message from the list */
                list_del(&pos->m_next);
                /* extracting the value */
                *value = pos->m_value;
                /* adding the element to the free list */
                list_add_tail(&pos->m_next, &message_h);
                return 0;
            }
        }
        /* nessun massaggio da parte di sender è stato trovato */
        return -1;
    }
}