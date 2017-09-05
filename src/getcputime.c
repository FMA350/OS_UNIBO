#include <mikabooq.h>

unsigned int
getcputime_s(const struct tcb_t *applicant)
{
    // tprintf("SSI: run time requested: %d\n",applicant->run_time);
    return applicant->run_time;
}
