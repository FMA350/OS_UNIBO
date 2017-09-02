inline unsigned int
wait_for_clock_s(struct tcb_t *applicant)
{
    thread_enqueue(applicant, t_wait4clock);
}
