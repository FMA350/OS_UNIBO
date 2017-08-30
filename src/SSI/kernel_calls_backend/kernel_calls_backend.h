
inline int get_errno_s(const struct tcb_t *applicant);

inline struct tcb_t *create_process_s(const state_t *initial_state, struct tcb_t *applicant);
inline struct tcb_t *create_thread_s(const state_t *initial_state, struct tcb_t *process);

inline void terminate_process_s(struct tcb_t *applicant);
inline void terminate_thread_s(struct tcb_t *thread);

inline struct tcb_t *setpgmmgr_s(struct tcb_t *thread, struct tcb_t *applicant, int *send_back);
inline struct tcb_t *settlbmgr_s(struct tcb_t *thread, struct tcb_t *applicant, int *send_back);
inline struct tcb_t *setsysmgr_s(struct tcb_t *thread, struct tcb_t *applicant, int *send_back);

inline unsigned int getcputime_s(const struct tcb_t *applicant);
inline unsigned int wait_for_clock_s(struct tcb_t *applicant);
inline unsigned int do_io_s(uintptr_t msgg, struct tcb_t* applic);

inline struct pcb_t *get_processid_s(const struct tcb_t *thread);
inline struct pcb_t *get_parentprocid_s(const struct pcb_t *proc);

void update_clock(unsigned int milliseconds);
