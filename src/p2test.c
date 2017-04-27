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

#include <uARMconst.h>
#include <uARMtypes.h>
#include <libuarm.h>

#include "const.h"
#include "listx.h"
#include "mikabooq.h"
#include "nucleus.h"
#include <stdint.h>

#define QPAGE 1024
#define TERM0ADDR               0x240

static struct tcb_t *printid;

static void ttyprintstring(devaddr device, char *s) {
	uintptr_t status;

	for (; *s; s++) {
		status = do_terminal_io(device, DEV_TTRS_C_TRSMCHAR | (*s << 8));
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
		sender = msgrecv(NULL, &payload);
		ttyprintstring(TERM0ADDR, (char *) payload);
		msgsend(sender, NULL);
	}
}

static inline void tty0print(char *s) {
	msgsend(printid, s);
	msgrecv(printid, NULL);
}

static inline void panic(char *s) {
	tty0print("!!! PANIC: ");
	tty0print(s);
	PANIC();
}

static struct tcb_t *csid;
#define CSIN msgsend(csid, NULL)
#define CSOUT msgsend(csid, NULL)

void cs_thread(void) {
	struct tcb_t *sender;
	for(;;){
		sender = msgrecv(NULL, NULL);
		msgrecv(sender, NULL);
	}
}

#define SYNCCODE 0x01000010

void p2(),p3(),p4(),p5(),p6(),p7(),p8();
struct tcb_t *testt,*p2t,*p3t,*p4t,*p5t,*p6t,*p7t,*p8t;

static state_t tmpstate;
memaddr stackalloc;

void test(void){
	ttyprintstring(TERM0ADDR, "NUCLEUS TEST: starting...\n");
	STST(&tmpstate);
	stackalloc = (tmpstate.sp + (QPAGE - 1)) & (~(QPAGE - 1));
	tmpstate.sp = (stackalloc -= QPAGE);
	tmpstate.pc = (memaddr)tty0out_thread;
	tmpstate.cpsr = STATUS_ALL_INT_ENABLE(tmpstate.cpsr);
	printid = create_thread(&tmpstate);
	tty0print("NUCLEUS: first msg printed by tty0out_thread\n");
	testt = get_mythreadid();

	tmpstate.sp = (stackalloc -= QPAGE);
	tmpstate.pc = (memaddr)cs_thread;
	csid = create_process(&tmpstate);
	tty0print("NUCLEUS: critial section thread started\n");

	CSIN;
	tmpstate.sp = (stackalloc -= QPAGE);
	CSOUT;
	tmpstate.pc = (memaddr)p2;
	p2t = create_process(&tmpstate);
	msgsend(p2t, SYNCCODE);
	msgrecv(p2t, NULL);

	CSIN;
	tmpstate.sp = (stackalloc -= QPAGE);
	CSOUT;
	tmpstate.pc = (memaddr)p3;
	p3t = create_process(&tmpstate);
	msgrecv(p3t, NULL);

	CSIN;
	tmpstate.sp = (stackalloc -= QPAGE);
	CSOUT;
	tmpstate.pc = (memaddr)p4;
	p4t = create_process(&tmpstate);
	msgsend(p4t, NULL);
	msgrecv(p4t, NULL);
	msgsend(p4t, tmpstate.sp);
	msgrecv(p4t, NULL);

	if (geterrno() == 0)
		panic("p1 wrong errno: recv from p4 should abort, p4 terminated\n");

	CSIN;
	tmpstate.sp = (stackalloc -= QPAGE);
	CSOUT;
	tmpstate.pc = (memaddr)p4;
	p5t = create_process(&tmpstate);

	CSIN;
	tmpstate.sp = (stackalloc -= QPAGE);
	CSOUT;
	tmpstate.pc = (memaddr)p4;
	p6t = create_process(&tmpstate);

	CSIN;
	tmpstate.sp = (stackalloc -= QPAGE);
	CSOUT;
	tmpstate.pc = (memaddr)p4;
	p7t = create_process(&tmpstate);

	CSIN;
	tmpstate.sp = (stackalloc -= QPAGE);
	CSOUT;
	tmpstate.pc = (memaddr)p4;
	p8t = create_process(&tmpstate);

	msgrecv(p8t, NULL);

	tty0print("NUCLEUS: test completed\n");
	HALT();
}

#define MINLOOPTIME             10000
#define LOOPNUM                 10000

void p2(void){
	struct tcb_t *p1t;
	uintptr_t value;
	cputime now1, now2;
	cputime cpu_t1, cpu_t2;
	int i;

	tty0print("p2 started\n");

	/* test: GET_MYTHREADID GET_PROCESSID GET_PARENTPROCID */
	if (get_mythreadid() != p2t)
		panic("p2 get_mythreadid: wrong pid returned\n");

	p1t = msgrecv(NULL, &value);
	if (value != SYNCCODE)
		panic("p2 recv: got the wrong value\n");
	if (p1t != testt)
		panic("p2 recv: got the wrong sender\n");
	if (get_processid(p1t) != get_parentprocid(get_processid(get_mythreadid())))
		panic("p2 get_parentprocid get_processid error\n");

	/* test: GET_CPUTIME */

	now1 = getTODLO();
	cpu_t1 = getcputime();
	/* delay for several milliseconds */
	for (i = 1; i < LOOPNUM; i++)
		;

	cpu_t2 = getcputime();
	now2 = getTODLO();

	if ((cpu_t2 - cpu_t1) >= MINLOOPTIME)
		tty0print("p2 GET_CPUTIME sounds okay\n");
	else
		panic("p2 GETCPUTIME sounds faulty\n");

	msgsend(p1t, NULL);

	terminate_thread();

	panic("p2 survived TERMINATE_THREAD\n");
}

void p3(void){
	tty0print("p3 started\n");

	cputime   time1, time2;
	while ((time2 - time1) < (CLOCKINTERVAL >> 1) )  {
		time1 = getTODLO();             /* time of day     */
		waitforclock();
		time2 = getTODLO();             /* new time of day */
	}
	tty0print("WAITCLOCK OK\n");

	msgsend(testt, NULL);

	terminate_process();

	panic("p3 survived TERMINATE_PROCESS\n");
}

void p4(void){
	static int p4inc = 1;
	struct tcb_t *parent;
	struct tcb_t *child;
	state_t p4childstate;

	switch (p4inc) {
		case 1:
			tty0print("first incarnation of p4 starts\n");
			break;
		case 2:
			tty0print("second incarnation of p4 starts\n");
			break;
	}
	p4inc++;
	parent = msgrecv(NULL, NULL);
	msgsend(parent, NULL);

	parent = msgrecv(NULL, &p4childstate.sp);
	/* only the first incarnation reaches this point */

	STST(&p4childstate);
	CSIN;
	p4childstate.sp = (stackalloc -= QPAGE);
	CSOUT;
	p4childstate.pc = (memaddr)p4;

	child = create_process(&p4childstate);
	msgsend(child, NULL);
	msgrecv(child, NULL);

	terminate_process();

	panic("p4 survived TERMINATE_PROCESS\n");
}

void p5a();

void p5p(void){
	uintptr_t errcode;
	struct tcb_t *sender;
	for(;;) {
		sender = msgrecv(NULL, &errcode);
		switch (errcode) {
			case PRIVINSTR:
				tty0print("p5 got PRIVINSTR\n");
				terminate_process();
		}
		msgsend(sender, NULL);
	}
}

void p5m(void){
	uintptr_t errcode;
	struct tcb_t *sender;
	for(;;) {
		sender = msgrecv(NULL, &errcode);
		switch (errcode) {
			default:
				tty0print("p5 mem error passup okay\n");
				sender->t_s.pc = (memaddr) p5a;
				break;
		}
		msgsend(sender, NULL);
	}
}

void p5s(void){
	uintptr_t sys[4];
	uintptr_t retval;
	struct tcb_t *sender;
	for(;;) {
		sender = msgrecv(NULL, sys);
		switch (sys[0]) {
			case 42:
				retval = 42;
				break;
			case 3:
				retval = 0;
				break;
			default:
				retval = -1;
				break;
		}
		msgsend(sender, retval);
	}
}

#define BADADDR 0xFFFFFFFF

void p5(void){
	struct tcb_t *parent;
	state_t mgrstate;
	uintptr_t retval;

	STST(&mgrstate);

	parent = msgrecv(NULL, &mgrstate.pc);

	CSIN;
	mgrstate.sp = (stackalloc -= QPAGE);
	CSOUT;
	mgrstate.pc = (memaddr)p5p;
	setpgmmgr(create_process(&mgrstate));

	CSIN;
	mgrstate.sp = (stackalloc -= QPAGE);
	CSOUT;
	mgrstate.pc = (memaddr)p5m;
	settlbmgr(create_process(&mgrstate));

	CSIN;
	mgrstate.sp = (stackalloc -= QPAGE);
	CSOUT;
	mgrstate.pc = (memaddr)p5s;
	setsysmgr(create_process(&mgrstate));

	/* this should me handled by p5s */

	retval = SYSCALL(42, 42, 42, 42);
	if (retval == 42)
		tty0print("p5 syscall passup okay\n");
	else
		panic("p5 syscall passup error\n");

	* ((memaddr *) BADADDR) = 0;

	panic("p5 survived mem error");
}

void p5a(void){
	uintptr_t retval;
	/* switch to user mode */
	setSTATUS((getSTATUS() & STATUS_CLEAR_MODE) | STATUS_USER_MODE);

	retval = SYSCALL(3, 2, 1, 0);
	if (retval == 0)
		tty0print("p5 user mode syscall passup okay\n");
	else
		panic("p5 user mode syscall passup error\n");

	/* this should fail! */
	msgsend(SSI, NULL);
}

void p6(void){
	*((memaddr *) BADADDR) = 0;

	panic("p6 survived mem error");
}

void p7(void){
	SYSCALL(3, 2, 1, 0);

	panic("p7 survived syscall without manager");
}

void p8child(), p8grandchild();
void p8(void){
	state_t childstate;
	struct tcb_t *pid = get_mythreadid();
	STST(&childstate);
	childstate.pc = (memaddr)p8child;
	CSIN;
	childstate.sp = (stackalloc -= QPAGE);
	CSOUT;
	msgsend(create_process(&childstate), pid);
	CSIN;
	childstate.sp = (stackalloc -= QPAGE);
	CSOUT;
	msgsend(create_process(&childstate), pid);
	msgrecv(NULL, NULL);
	msgrecv(NULL, NULL);
	msgrecv(NULL, NULL);
	msgrecv(NULL, NULL);

	msgsend(testt, NULL);

	terminate_process();

	panic("p8 survived TERMINATE_PROCESS\n");
}

#define NGRANDCHILDREN 3
void p8child() {
	int i;
	state_t childstate;
	struct tcb_t *ppid;
	msgrecv(NULL, &ppid);
	for (i=0; i<NGRANDCHILDREN; i++) {
		STST(&childstate);
		childstate.pc = (memaddr)p8grandchild;
		CSIN;
		childstate.sp = (stackalloc -= QPAGE);
		CSOUT;
		msgsend(create_process(&childstate), ppid);
	}
	msgrecv(NULL, NULL);

	panic("p8child survived parent termination\n");
}

void p8grandchild() {
	struct tcb_t *gpid;
	msgrecv(NULL, &gpid);
	msgsend(gpid, NULL);
	msgrecv(NULL, NULL);

	panic("p8grandchild survived grandparent termination\n");
}
