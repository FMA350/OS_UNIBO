#include <scheduler.h>
#include <libuarm.h>
#include <ssi.h>
#include <nucleus.h>
#include <trap.h>

extern struct list_head blockedq;




void tlbtrap_h() {
	/*tprint("tlbtrap_h started\n");  
	thread_enqueue(current_thread, &blockedq);
	if (current_thread->t_pcb->tlb_mgr == NULL)
	{
		struct {
			uintptr_t reqtag;
		} req = {TERMINATE_PROCESS};
		msgsend(SSI, &req);
	}else{
		int exc_cause = getCAUSE();
		msgsend(current_thread->t_pcb->tlb_mgr, exc_cause);
	}
	scheduler();*/
	
	handleTrap(tlb_mgr);
	
}
