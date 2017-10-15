#include <mikabooq.h>
#include "do_io.h"
#include <scheduler.h>
#include <syslib.h>


struct tcb_t *soft_blocked_thread[N_EXT_IL][N_DEV_PER_IL] = { [0 ... 4][0 ... 7] = NULL };

void dtpe_request(devaddr device, uintptr_t command, uintptr_t data0, uintptr_t data1);
void terminal_request(devaddr device, uintptr_t command);


void do_io_s(devaddr device, uintptr_t command, uintptr_t data0,
                            uintptr_t data1, struct tcb_t* applicant)
{
    // external devices interrupt line (0 == disk)
    unsigned int line = (device - 0x40) / 0x80;
    // device number
    unsigned int dev = (device - 0x40 - line * 0x80) / 0x10;

    assert(line >= 0 && line <= 4);
    assert(dev >= 0 && dev <= 7);

    if (soft_blocked_thread[line][dev]) {
    // Qualcun' altro sta già facendo IO; non dovrebbe mai accadere
        tprint("SSI - do_io_s: device busy");
        PANIC();
    }

    soft_blocked_thread[line][dev] = applicant;
    // the thread gets soft blocked
    soft_block_count++;

    switch (line) {
        case EXT_IL_INDEX(IL_TERMINAL):
            terminal_request(device, command);
            break;
        case EXT_IL_INDEX(IL_DISK):
        case EXT_IL_INDEX(IL_TAPE):
        case EXT_IL_INDEX(IL_PRINTER):
        case EXT_IL_INDEX(IL_ETHERNET):
            dtpe_request(device, command, data0, data1);
            break;
        default:
            tprint("SSI - do_io_s: IO ERROR\n");
            PANIC();
    }
}

// disks, tapes, printers and ethernet
void dtpe_request(devaddr device, uintptr_t command, uintptr_t data0, uintptr_t data1)
{
    *((unsigned int *) (device + 0x8)) = data0;
    *((unsigned int *) (device + 0xC)) = data1;
    *((unsigned int *) (device + 0x4)) = command;
}

void terminal_request(devaddr device, uintptr_t command)
{
    *((unsigned int *) (device + 0x4)) = command;
}
