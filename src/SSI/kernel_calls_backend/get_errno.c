
inline int get_errno_s(const struct tcb_t *applicant)
{
    return applicant->errno;
}
