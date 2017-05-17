#ifndef TRAP_H
#define TRAP_H

#define handleTrap(mng_name)({  													 \
	tprint("mng_name started\n"); 												 \
	thread_enqueue(current_thread, &blockedq);				 \
	if (current_thread->t_pcb->mng_name == NULL) 					\
	{ 																								\
		struct {																				 \
			uintptr_t reqtag;															 \
		} req = {TERMINATE_PROCESS}; 										\
		msgsend(SSI, &req);														 \
	}else{ 																						\
		int exc_cause = getCAUSE(); 												\
		msgsend(current_thread->t_pcb->mng_name, exc_cause); 		\
	}																								 \
	scheduler();																					 \
})

#endif