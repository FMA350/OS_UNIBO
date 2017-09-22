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


static inline clean_mgr(struct pcb_t *proc);


#define __qhead(queue, type, member) ({                         \
    struct list_head *new_head = list_next(queue);              \
    new_head ? container_of(new_head, type, member) : NULL;     \
})


/* member è il campo con cui la lista è collegata */
#define __dequeue(queue, type, member) ({                                      \
    struct list_head *new_head = list_next(queue);                          \
    new_head ? ({list_del(new_head);                                       \
                container_of(new_head, type, member);}) : NULL;     \
})


struct pcb_t *proc_init(void) {

    /* process[0] is the root process */
    process[0].p_parent = NULL;
    INIT_LIST_HEAD(&process[0].p_threads);
    INIT_LIST_HEAD(&process[0].p_children);
    INIT_LIST_HEAD(&process[0].p_siblings);
    clean_mgr(process);

    size_t i;
    for (i = 1; i < MAXPROC; i++) {
        /* the sentinel of the free list is process[0].p_siblings */
        list_add_tail(&process[i].p_siblings, &(process[0].p_siblings));
        process[i].p_parent = NULL;
        INIT_LIST_HEAD(&process[i].p_threads);
        INIT_LIST_HEAD(&process[i].p_children);
        clean_mgr(process + i);
    }
    return process;
}

static inline void proc_enqueue(struct pcb_t *new, struct list_head *queue) {
    list_add_tail(&new->p_siblings, queue);
}

struct pcb_t *proc_dequeue(struct list_head *queue) {
    return __dequeue(queue, struct pcb_t, p_siblings);
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
    proc_enqueue(new_proc, &p_parent->p_children);

    return new_proc;
}

/* delete a process (properly updating the process tree links) */
/* this function must fail if the process has threads or children. */
/* return value: 0 in case of success, -1 otherwise */

int proc_delete(struct pcb_t *oldproc){
    if (oldproc->p_parent == NULL ||                // Trying to delete root or a non-allocated process
        !list_empty(&oldproc->p_children) ||        // Trying to delete a process with children
        !list_empty(&oldproc->p_threads))           // Trying to delete a process with threads
        return -1;
    else {
        // the process can be deleted
        oldproc->p_parent = NULL;
        list_del(&oldproc->p_siblings);
        list_add_tail(&oldproc->p_siblings, &(process[0].p_siblings));
        clean_mgr(oldproc);

        return 0;
    }
}

/* return the pointer to the first child (NULL if the process has no children) */
/*
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
*/
inline struct pcb_t *proc_firstchild(struct pcb_t *proc) {
    return __qhead(&proc->p_children, struct pcb_t, p_siblings);
}
/*
inline struct tcb_t *proc_firstthread(struct pcb_t *proc){
    struct list_head *first_thread = list_next(&proc->p_threads);

    if (first_thread)
        // the process has at least one thread
        return container_of(first_thread, struct tcb_t, t_next);
    else
        return NULL;
}*/

inline struct tcb_t *proc_firstthread(struct pcb_t *proc) {
    return __qhead(&proc->p_threads, struct tcb_t, t_next);
}

static inline clean_mgr(struct pcb_t *proc) {
    proc->pgm_mgr = proc->tlb_mgr = proc->sys_mgr = NULL;
}

/****************************************** THREAD ALLOCATION ****************/

static inline clean_thread_data(struct tcb_t *thread);

void thread_init(void) {
    size_t i;
    for (i = 0; i < MAXTHREAD; i++) {
        clean_thread_data(thread + i);

        list_add_tail(&thread[i].t_next, &thread_h); //collego i vari elementi della lista libera
        // INIT_LIST_HEAD(&thread[i].t_sched);
        INIT_LIST_HEAD(&thread[i].t_msgq);
        INIT_LIST_HEAD(&thread[i].t_wait4me);
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

    if(!list_empty(&oldthread->t_msgq))
    // the thread shouldn't have message
        return -1; //there are messsages left in the queue.
    if (!list_empty(&oldthread->t_wait4me))
    // the thread shouldn't have other threads waiting for him
        return -2;

    clean_thread_data(oldthread);

    //remove the thread from the process queue
    list_del(&oldthread->t_next);
    list_del(&oldthread->t_sched);
    //t_msgq and t_wait4me are already empty.

    /*adding oldthread to the free list*/
    list_add_tail(&oldthread->t_next, &thread_h);
    return 0;
}

static inline clean_thread_data(struct tcb_t *thread) {
    thread->t_pcb = NULL;
    thread->t_status = T_STATUS_NONE;
    thread->t_wait4sender = NULL;

    thread->run_time = thread->errno = 0;
}

/*************************** THREAD QUEUE ************************/


/* add a tcb to the scheduling queue */
void thread_enqueue(struct tcb_t *new, struct list_head *queue) { //chopperEdit
    list_add_tail(&new->t_sched, queue);
}

/* return the head element of a scheduling queue.
	 (this function does not dequeues the element)
	 return NULL if the list is empty */
// struct tcb_t *thread_qhead(struct list_head *queue) { //chopperEdit
//     struct list_head *new_head = list_next(queue);
//     if(new_head == NULL)
//         return NULL;
//     else
//         /* t_next ---> t_sched */
//         return container_of(new_head, struct tcb_t, t_sched);
// }

struct tcb_t *thread_qhead(struct list_head *queue) {
    return __qhead(queue, struct tcb_t, t_sched);
}

/* get the first element of a scheduling queue.
	 return NULL if the list is empty */
/*
struct tcb_t *thread_dequeue(struct list_head *queue) {
    struct list_head *new_head = list_next(queue);
    if(new_head == NULL)
        return NULL;
    else {
        list_del(new_head);
        return container_of(new_head, struct tcb_t, t_sched);
    }
}
*/
struct tcb_t *thread_dequeue(struct list_head *queue) {
    struct tcb_t *thread = __dequeue(queue, struct tcb_t, t_sched);
    (&thread->t_sched)->next = NULL;
    (&thread->t_sched)->prev = NULL;
    return thread;
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

int msgq_add_head(struct tcb_t *sender, struct tcb_t *destination, uintptr_t value) {
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
        list_add(new_space, &destination->t_msgq);

        return 0;
    }
}

// stores the value only if value != NULL
static inline store_val(uintptr_t *value, struct msg_t *msg) {
    if (value)
    // value != NULL
        /* storing the vaue */
        *value = msg->m_value;
}

int msgq_get(struct tcb_t **sender, struct tcb_t *destination, uintptr_t *value) {
    if(sender == NULL || *sender == NULL) {
        /* restituisce il primo messaggio in coda qualsiasi ne sia il mittente
         * L’indirizzo del TCB del mittente viene memorizzato in *sender */

        //concatenatore del primo messaggio in coda
        struct list_head *msg_conc = NULL;
        if (destination != NULL)
            msg_conc = list_next(&destination->t_msgq);
        if (msg_conc == NULL)
            /* empty queue */
            return -1;
        else {
            struct msg_t *msg = container_of(msg_conc, struct msg_t, m_next);
            store_val(value, msg);
            if (sender)
            /* sender != NULL && *sender == NULL */
                /* store the sender */
                *sender = msg->m_sender;
            /* freeing the message */
            msg_free(msg);
            return 0;
        }
    } else {
    /* sender != NULL && *sender != NULL */
    /* restituisce il primo messaggio in coda che ha *sender come mittente */
        struct msg_t *pos;
        if (destination != NULL){ //BUG FIXED
            list_for_each_entry(pos, &destination->t_msgq, m_next) {
                if(pos->m_sender == *sender) {
                    store_val(value, pos);
                    /* freeing the message */
                    msg_free(pos);
                    return 0;
                }
            }
        }
        /* nessun massaggio da parte di sender è stato trovato */
        return -1;
    }
}

int msg_free(struct msg_t *oldmsg) {
    //remove the msg from the process queue.
    list_del(&oldmsg->m_next);
    oldmsg->m_sender = NULL;

    list_add_tail(&oldmsg->m_next, &message_h);
    return 0;
}

struct msg_t *msg_qhead(struct list_head *queue) {
    return __qhead(queue, struct msg_t, m_next);
}
 /*
struct msg_t *msg_qhead(struct list_head *queue) {
    struct list_head *new_head = list_next(queue);
    if(new_head == NULL)
        return NULL;
    else
        return container_of(new_head, struct msg_t, m_next);
}
*/
