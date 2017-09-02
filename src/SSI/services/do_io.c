
void do_io_s(devaddr device, uintptr_t command, uintptr_t data1,
                            uintptr_t data2, struct tcb_t* applicant)
{
    if (command == DEV_C_ACK) {
    // se è un acknoledgement
        if (soft_blocked_thread[TERMINAL_REQUESTER_INDEX]) {
        // DEBUG: interrupt proveniente non da tprint
            // continue;
            // tprint("SSI: Requester is NULL\n");
            // PANIC();

            msgsend(soft_blocked_thread[TERMINAL_REQUESTER_INDEX],
                *((unsigned int *) TERMINAL_DEV_FIELD(0, TRANSM_STATUS)));

            // TODO: mnalli - l'ho aggiunto io, è giusto?
            // tprintf("soft_block_count == %d\n", soft_block_count);
            soft_block_count--;
            soft_blocked_thread[TERMINAL_REQUESTER_INDEX] = NULL;
        }
    }

    switch (device) {
        case TERMINAL_DEV_FIELD(0, TRANSM_COMMAND):   //il device e' un terminale
            // the thread gets soft blocked
            soft_block_count++;

            // setdevice(0, command);
            *((uintptr_t *) TERMINAL_DEV_FIELD(0, TRANSM_COMMAND)) = command;

            if (soft_blocked_thread[TERMINAL_REQUESTER_INDEX])
            // Qualcun altro sta già facendo IO; non dovrebbe mai accadere
                PANIC();

            soft_blocked_thread[TERMINAL_REQUESTER_INDEX] = applicant;

            break;
        // case PRINTADDR:
        //     break;
        // case NETADDR:
        //     break;
        // case TAPEADDR:
        //     break;
        // case DISKADDR:
        //     break;
        default:
            // tprint("SSI: IO ERROR\n");
            PANIC();
    }
}
