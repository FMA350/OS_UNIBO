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
