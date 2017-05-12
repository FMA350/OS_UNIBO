#ifndef MIKABOOQ_H
#define MIKABOOQ_H
#include <listx.h>
#include <sys/types.h>
#include <uARMtypes.h>

struct pcb_t {
	struct pcb_t *p_parent ; /* pointer to parent */
	struct list_head p_threads; /* list of threads */

	struct list_head p_children; /* list of children (hierarchy of processes) */
	struct list_head p_siblings; /* link the other siblings (children of p_parent) */

	struct tcb_t *pgm_mgr;
	struct tcb_t *tlb_mgr;
	struct tcb_t *sys_mgr;
};

#define T_STATUS_NONE  0    /* unused thread descriptor */
#define T_STATUS_READY 1
#define T_STATUS_W4MSG 4

// TODO: update initialization of threads
// FIXME: not every INIT_LIST_HEAD(...) we have placed is necessary during initialization

struct tcb_t {
	struct pcb_t *t_pcb; /* pointer to the process */
	state_t t_s; /* processor state */

	int t_status;
	unsigned int run_time; //milliseconds of CPU time used. check scheduler.c for the accounting function
	int errno; /* error status of the thread */

	struct tcb_t *t_wait4sender; /* expected sender (if t_status == T_STATUS_W4MSG), NULL means accept msg from anybody */
	struct list_head t_next; /* link the other elements of the list of threads in the same process */
	struct list_head t_sched; /* link the other elements on the same scheduling list */
	struct list_head t_msgq; /* list of pending messages for the current thread */

	struct list_head t_wait4me; /* list of threads waiting for a message from the current thread */
	struct list_head t_wait4same; /* link the other element on the same t_wait4me list */
};

struct msg_t {
	struct tcb_t *m_sender; /* sender thread */
	uintptr_t m_value; /* payload of the message */
	struct list_head m_next; /* link the other elements of the pending message queue */
};

/************************************** PROC MGMT ************************/

/* initialize the data structure */
/* the return value is the address of the root process */
struct pcb_t *proc_init(void);

/* alloc a new empty pcb (as a child of p_parent) */
/* p_parent cannot be NULL */
/* return NULL if p_parent == NULL or there are no more element free */
struct pcb_t *proc_alloc(struct pcb_t *p_parent);


/* delete a process (properly updating the process tree links) */
/* this function must fail if the process has threads or children. */
/* return value: 0 in case of success, -1 otherwise */
int proc_delete(struct pcb_t *oldproc);

/* return the pointer to the first child (NULL if the process has no children) */
struct pcb_t *proc_firstchild(struct pcb_t *proc);

/* return the pointer to the first thread (NULL if the process has no threads) */
struct tcb_t *proc_firstthread(struct pcb_t *proc);


/****************************************** THREAD ALLOCATION ****************/

/* initialize the data structure */
void thread_init(void);

/* alloc a new tcb (as a thread of process) */
/* return -1 if process == NULL or mo more available tcb-s.
	 return 0 (success) otherwise */
struct tcb_t *thread_alloc(struct pcb_t *process);

/* Deallocate a tcb (unregistering it from the list of threads of
	 its process) */
/* it fails if:
	 the message queue is not empty (returning -1)
	 the list of threads waiting for the current thread is not empty (returning -2)*/
int thread_free(struct tcb_t *oldthread);

/*************************** THREAD QUEUE ************************/

/* add a tcb to the scheduling queue */
void thread_enqueue(struct tcb_t *new, struct list_head *queue);

/* return the head element of a scheduling queue.
	 (this function does not dequeues the element)
	 return NULL if the list is empty */
struct tcb_t *thread_qhead(struct list_head *queue);

/* get the first element of a scheduling queue.
	 return NULL if the list is empty */
struct tcb_t *thread_dequeue(struct list_head *queue);

static inline void thread_outqueue(struct tcb_t *this) {
	list_del(&this->t_sched);
}

#define for_each_thread_in_q(pos, queue) \
	list_for_each_entry(pos, queue, t_sched)

/*************************** MSG QUEUE ************************/

/* initialize the data structure */
void msgq_init(void);

/* add a message to a message queue. */
/* msgq_add fails (returning -1) if no more msgq elements are available */
int msgq_add(struct tcb_t *sender, struct tcb_t *destination, uintptr_t value);

/* same as msgq_add, except from the fact that the message is added in the head */
int msgq_add_head(struct tcb_t *sender, struct tcb_t *destination, uintptr_t value);

int msg_free(struct msg_t *oldmsg);

struct msg_t *msg_qhead(struct list_head *queue) ;

/* retrieve a message from a message queue */
/* -> if sender == NULL: get a message from any sender
	 -> if sender != NULL && *sender == NULL: get a message from any sender and store
	 the address of the sending tcb in *sender
	 -> if sender != NULL && *sender != NULL: get a message sent by *sender */
/* -> if value == NULL: the message is not stored */
/* return -1 if there are no messages in the queue matching the request.
	 return 0 and store the message payload in *value otherwise. */
int msgq_get(struct tcb_t **sender, struct tcb_t *destination, uintptr_t *value);

/*************************** WAITING4MSG LIST *************************************/


/* add a tcb to dest's t_wait4me list */
void wait4thread_add(struct tcb_t *dest, struct tcb_t *waiting);

/* remove this from the list of threads waiting for the same he was waiting to:
   should be used when resuming a thread */
void wait4thread_del(struct tcb_t *this);


struct tcb_t *wait4thread_qhead(struct list_head *queue);

struct tcb_t *wait4thread_dequeue(struct list_head *queue);



#endif
