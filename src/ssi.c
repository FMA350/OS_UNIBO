#include "ssi.h"

#include <listx.h>
#include <uARMtypes.h>
#include <libuarm.h>
#include <syslib.h>
#include <mikabooq.h>
#include <nucleus.h>
#include <scheduler.h>

#include "handlers.h"
#include "services.h"
#include "do_io.h"


struct tcb_t *SSI;

struct tcb_t *ssi_thread_init(void)
{
    static struct tcb_t _SSI;

    _SSI.t_pcb = NULL;
    _SSI.t_status = T_STATUS_READY;
    _SSI.t_wait4sender = NULL;
    _SSI.t_s.cpsr = STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE);
    INIT_LIST_HEAD(&_SSI.t_msgq);
    INIT_LIST_HEAD(&_SSI.t_wait4me);

    //tprintf("SSI initialized: %p\n", _SSI);

    return(SSI = &_SSI);
}

static inline uintptr_t req_field(uintptr_t request, int i)
{
    return ((uintptr_t *) request)[i];
}

void ssi(void)
{
    while (1) {
        uintptr_t msg, reply;
        int send_back;

        struct tcb_t *applicant = msgrecv(NULL, &msg);

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
                reply = (uintptr_t) getcputime_s(applicant);
                BREAKPOINT();
                msgsend(applicant, reply);
                break;
            case WAIT_FOR_CLOCK:
                wait_for_clock_s(applicant);
                break;
            case DO_IO:
                do_io_s((devaddr) req_field(msg, 1),
                        (uintptr_t) req_field(msg, 2),
                        (uintptr_t) req_field(msg, 3),
                        (uintptr_t) req_field(msg, 4), applicant);
                break;
            case GET_PROCESSID:
                msgsend(applicant, (uintptr_t) get_processid_s((struct tcb_t *) req_field(msg, 1)));
                break;
            case GET_PARENTPROCID:
                msgsend(applicant, (uintptr_t) get_parentprocid_s((struct pcb_t *) req_field(msg, 1)));
                break;
            case GET_MYTHREADID:
                msgsend(applicant, (uintptr_t) applicant);
                break;
            default:
                tprint("SSI: Unknown request\n");
                tprintf("REQ_TAG == %d, msg == %p, applicant = %p\n",
                        req_field(msg, 0), msg, applicant);
                PANIC();
                break;
            // TODO: se il messaggio è diverso dai codici noti
            //       rispondere con errore e settare errno
        }
    }
}
