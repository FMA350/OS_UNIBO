
// sentinella della coda dei processi richiedenti il servizio di attesa
LIST_HEAD(t_wait4clock);

int pseudoclock = 0;

unsigned int
wait_for_clock_s(struct tcb_t *applicant)
{
    // TODO: bisogna specificare la lista del thread
    thread_enqueue(applicant, &t_wait4clock);
}
