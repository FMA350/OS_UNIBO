
extern struct list_head t_wait4clock;

void pseudoclock_init(void);

void pseudoclock_check(void);

uint32_t pseudoclock_time_to_next_tick(void);
