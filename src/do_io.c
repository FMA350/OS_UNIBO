#include <mikabooq.h>
#include "do_io.h"
#include <scheduler.h>


struct tcb_t *soft_blocked_thread[N_EXT_IL][N_DEV_PER_IL] = { [0 ... 4][0 ... 7] = NULL };


void do_io_s(devaddr device, uintptr_t command, uintptr_t data1,
                            uintptr_t data2, struct tcb_t* applicant)
{
    int dev = ((device - 0x4c) / 0x80); //dato l'indirizzo calcolo qual'e il device
    int line = (device - 0x4c - (dev * 0x80))/ 0x10; //dato l'indirizzo calcolo qual'e il numero del device
    //tprintf("devno = %d, line = %d\n", dev, line);
    switch (dev) {
        case EXT_IL_INDEX(IL_TERMINAL):
             // the thread gets soft blocked
            soft_block_count++;
            // Setting command
            *((uintptr_t *) TERMINAL_DEV_FIELD(line, TRANSM_COMMAND)) = command;
            if (soft_blocked_thread[dev][line]) {
            // Qualcun altro sta già facendo IO; non dovrebbe mai accadere
                tprint("SSI - do_io_s: device busy");
                PANIC();
            }

            soft_blocked_thread[dev][line] = applicant;
            break;

        case EXT_IL_INDEX(IL_DISK):
        case EXT_IL_INDEX(IL_TAPE):
        case EXT_IL_INDEX(IL_PRINTER):
        case EXT_IL_INDEX(IL_ETHERNET):

        default:
            tprint("SSI - do_io_s: IO ERROR\n");
            PANIC();
    }
}
