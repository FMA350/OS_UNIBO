#include <mikabooq.h>

uint64_t getcputime_s(const struct tcb_t *applicant)
{
    // tprintf("SSI: run time requested: %d\n",(int) applicant->run_time);
    return applicant->run_time;
}
