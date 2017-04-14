#include <libuarm.h>
#include <sys/types.h>

extern void BREAKPOINT();


void test_wait() {
    tprint(" The test has started\n"
           " Ready to wait\n");
    BREAKPOINT();
    WAIT();
}

void test_syscall() {
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


void test_timer() {
    tprint("test_timer started\n");
    register size_t i, j;
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 100; j++) {
            WAIT();
        }
        BREAKPOINT();
        tprint("100 done\n");
    }
    tprint("terminating test_timer\n");
}
