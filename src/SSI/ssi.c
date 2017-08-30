#include <listx.h>
#include <ssi.h>
#include <uARMtypes.h>
#include <libuarm.h>
#include <syslib.h>
#include <mikabooq.h>
#include <nucleus.h>
#include <scheduler.h>
#include <syscall.h>
#include <interrupts.h>

#include "kernel_calls_backend/kernel_calls_backend.h"

#define TERM0ADDR       0x24C
#define PRINTADDR       0x1C0
#define NETADDR         0x140
#define TAPEADDR        0x0C0
#define DISKADDR        0x040

struct io_req{
    uintptr_t val;
    struct tcb_t *requester;
};
struct io_req request[8];

struct list_head *t_wait4clock;
int pseudoclock;

struct tcb_t *SSI , *IO_thread;


struct tcb_t *ssi_thread_init() {
    static struct tcb_t _SSI;

    _SSI.t_pcb = NULL;
    _SSI.t_status = T_STATUS_READY;
    _SSI.t_wait4sender = NULL;
    _SSI.t_s.cpsr = STATUS_ALL_INT_DISABLE(_SSI.t_s.cpsr);
    INIT_LIST_HEAD(&_SSI.t_msgq);
    INIT_LIST_HEAD(&_SSI.t_wait4me);
    INIT_LIST_HEAD(t_wait4clock);
    pseudoclock = 0;

    int i;
    for (i=0; i<8; i++)
        request[i].requester = NULL;

    tprint("SSI initialized\n");

    return(SSI = &_SSI);
}

static inline uintptr_t req_field(uintptr_t request, int i) {
    return ((uintptr_t *) request)[i];
}

void ssi(){
    while (1) {
        uintptr_t msg, reply;
        int send_back;
        struct tcb_t *applicant = msgrecv(NULL, &msg);
        // tprintf("SSI request:%d\n", req_field(msg,0));
        //         "   applicant == %p\n"
        //         "   request number == %d\n"
        //         "   headOfReadyQ == %p\n",
        //        applicant, req_field(msg, 0), current_thread, thread_qhead(&readyq));

        void * IO_addr = (void *) 0x00006ff0; //FIXME: move to a declaration.

        if(applicant == IO_addr) { //interrupt_h ci sta dicendo che un device ha completato
            int i = 0;
            while (request[i].requester==NULL && i<8)
                i++;
            void * rcv_status = (void *) 0x0000248;
            //tprintf("%p, %p, %p, recv status:%d, trs status:\n",current_thread, thread_qhead(&SSI->t_wait4me), request[i].requester,*(unsigned int*)rcv_status);
            msgsend(request[i].requester,*(unsigned int*)rcv_status);
            request[i].val = (uintptr_t) NULL;
            request[i].requester = NULL;
        }
        else {
            //it's a request from a thread
            switch (req_field(msg, 0)) {
                case GET_ERRNO:
                    msgsend(applicant, (uintptr_t) get_errno_s(applicant));
                break;
                case CREATE_PROCESS:
                    msgsend(applicant, (uintptr_t) create_process_s((state_t *) req_field(msg, 1), applicant));
                break;
                case CREATE_THREAD:
                    msgsend(applicant, (uintptr_t) create_thread_s((state_t *) req_field(msg, 1), applicant));
                break;
                case TERMINATE_PROCESS:
                    terminate_process_s(applicant);
                break;
                case TERMINATE_THREAD:
                    terminate_thread_s(applicant);
                break;
                case SETPGMMGR:
                    reply = (uintptr_t) setpgmmgr_s((struct tcb_t *) req_field(msg, 1), applicant, &send_back);
                    if (send_back)
                        msgsend(applicant, reply);
                break;
                case SETTLBMGR:
                    reply = (uintptr_t) settlbmgr_s((struct tcb_t *) req_field(msg, 1), applicant, &send_back);
                if (send_back)
                    msgsend(applicant, reply);
                break;
                case SETSYSMGR:
                    reply = (uintptr_t) setsysmgr_s((struct tcb_t *) req_field(msg, 1), applicant, &send_back);
                if (send_back)
                    msgsend(applicant, reply);
                break;
                case GET_CPUTIME:
                    msgsend(applicant, (uintptr_t) getcputime_s(applicant));
                break;
                case WAIT_FOR_CLOCK:
                    wait_for_clock_s(applicant);
                break;
                case DO_IO:
                    do_io_s(msg, applicant);
                break;
                case GET_PROCESSID:
                    msgsend(applicant, (uintptr_t) get_processid_s((struct tcb_t *) req_field(msg, 1)));
                break;
                case GET_PARENTPROCID:
                    msgsend(applicant, (uintptr_t) get_parentprocid_s((struct pcb_t *) req_field(msg, 1)));
                break;
                case GET_MYTHREADID:
                    msgsend(applicant, (uintptr_t) get_mythreadid_s((struct pcb_t *) req_field(msg, 1)));
                break;
                default:
                // TODO: se il messaggio è diverso dai codici noti
                //       rispondere con errore e settare errno
                break;
            }
        }
    }
}

/****************************************************************************/


#define PSEUDOCLOCK_TICK 100

void update_clock(unsigned int milliseconds){
    pseudoclock += milliseconds;
    if(pseudoclock >= PSEUDOCLOCK_TICK){
        pseudoclock -= PSEUDOCLOCK_TICK;
        struct tcb_t *to_resume;
        while((to_resume = thread_dequeue(t_wait4clock))!=NULL){
            msgsend(to_resume,NULL);
        }
    }
}
