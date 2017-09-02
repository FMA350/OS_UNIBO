#include "do_io.h"

struct tcb_t *soft_blocked_thread[5] = { [0 ... 4] = NULL };

void do_io_s(devaddr device, uintptr_t command, uintptr_t data1,
                            uintptr_t data2, struct tcb_t* applicant)
{
    // Per ora funziona solamente il terminale 0
    switch (device) {
        case TERMINAL_DEV_FIELD(0, TRANSM_COMMAND):   //il device e' un terminale
            // the thread gets soft blocked
            soft_block_count++;

            // Setting command
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
