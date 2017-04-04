#include "../const.h"
#include "../mikabooq.h"
#include "../listx.h"
/*
pages 16...23 of the manual

When handling Fast Interrupt Requests, the processor enters Fast Interrupt mode
with all interrupts disabled and Link Return register points to the address of the in-
struction that was not executed plus 4
[...]

When writing Exception Handlers code, it is well advised to pay attention to the Program
Counter value stored in the Old Area. As described above, each exception leaves a
different value in Link Return register and this value is automatically moved to Old
Area.pc by the ROM Level Exception Handlers, so, for example, when handling an
Interrupt, the Old Area.pc has to be decreased by 4 to point to the right return
instruction

  //state_t
typedef struct {
unsigned int a1;
//r0
unsigned int a2;
//r1
unsigned int a3;
//r2
unsigned int a4;
//r3
unsigned int v1;
//r4
unsigned int v2;
//r5
unsigned int v3;
//r6
unsigned int v4;
//r7
unsigned int v5;
//r8
unsigned int v6;
//r9
unsigned int sl;
//r10
unsigned int fp;
//r11
unsigned int ip;
//r12
unsigned int sp;
//r13
unsigned int lr;
//r14
unsigned int pc;
//r15
unsigned int cpsr;
unsigned int CP15 Control;
unsigned int CP15 EntryHi;
unsigned int CP15 Cause;
unsigned int TOD Hi;
unsigned int TOD Low;
} state t;

Each time an exception is risen, the BIOS handlers will store the processor state
before the exception into the proper Old area, perform other tasks where required (see
sec. 2.5.2) and eventually load the processor state stored in the corresponding New area.
The New areas must be filled with valid processor states pointing to kernel level
exception handlers by kernel initialization stage.
Syscall New   0x0000.7268
Syscall Old   0x0000.7210
PGMT New      0x0000.71B8
PGMT Old      0x0000.7160
TLB New       0x0000.7108
TLB Old       0x0000.70B0
Interrupt New 0x0000.7058
Interrupt Old 0x0000.7000








/*

static LIST_HEAD(ready_queue); //holds the queue


//where should we save the state of the processor?

int append(){
  //should we check if the thread is already present in the list?


}

void tick(){
  /** everytime the subroutine tick() is called
    * the next thread is put into execution
  */
}
