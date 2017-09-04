#include <mikabooq.h>
#include "accounting.h"

unsigned int
wait_for_clock_s(struct tcb_t *applicant)
{
    // TODO: bisogna specificare la lista del thread
    thread_enqueue(applicant, &t_wait4clock);
}
