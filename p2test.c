#include <libuarm.h>

extern void BREAKPOINT();

void test_wait(){
    tprint(" The test has started\n"
           " Ready to wait\n"
        );
    BREAKPOINT();
    WAIT();
}

void test_syscall(){
    tprint("test_syscall has started\n");
    BREAKPOINT();
    tprint("First syscall...\n"
           "All arguments are 1\n");
    SYSCALL(1, 1, 1, 1);
    tprint("Second syscall...\n"
           "All arguments are 0\n");
    SYSCALL(0, 0, 0, 0);
    tprint("Third syscall...\n"
           "First two args are 1 the others are 0\n");
    SYSCALL(1, 1, 0, 0);
    HALT();
}



/*
 * Copyright (C) 2017 Renzo Davoli
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
#include <uARMconst.h>
#include <uARMtypes.h>
#include <libuarm.h>

#include "const.h"
#include "listx.h"
#include "mikabooq.h"
#include "nucleus.h"
#include <stdint.h>

#define TERM0ADDR               0x240

static state_t printstate;
static struct tcb_t *printid;

static void ttyprintstring(void *device, char *s) {
	struct {
		uintptr_t reqtag;
		void *device;
		uintptr_t command;
	} req = { DO_IO, device, 0};
	uintptr_t status;

	for (; *s; s++) {
		req.command = TRANSMITCHAR | (*s << 8);
		msgsend(SSI, req);
		msgrecv(SSI, &status);
		switch (status & 0xff) {
			case DEV_S_READY:
			case DEV_TTRS_S_CHARTRSM: break;
			default: return;
		}
	}
}

void tty0out_thread(void) {
	uintptr_t payload;
	struct tcb_t *sender;

	for(;;){
		sender = MsgRecv(NULL, &payload);
		ttyprintstring(TERM0ADDR, (char *) payload);
		msgsend(sender, 0);
	}
}

void test(void){
	ttyprintstring(TERM0ADDR, "NUCLEUS TEST: starting...\n");
	STST(&printstate);
	printstate.s_sp = printstate.s_sp - QPAGE;
	printstate.s_pc = printstate.s_t9 = (memaddr)printthread;
}

*/
