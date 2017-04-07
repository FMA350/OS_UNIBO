
void states_init(){
    //TODO: complete loading and care for execution mode

    ((state_t *) INT_NEWAREA)->pc = (unsigned int) interrupt_h;
    ((state_t *) INT_NEWAREA)->cpsr = STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE);
    ((state_t *) INT_NEWAREA)->sp = RAM_TOP;

    ((state_t *) SYSBK_NEWAREA)->pc = (unsigned int) syscall_h;
    ((state_t *) SYSBK_NEWAREA)->cpsr = STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE);
    ((state_t *) SYSBK_NEWAREA)->sp = RAM_TOP;
}

void interrupt_h() {
    /* code */
}
