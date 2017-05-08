#include <stdio.h>
#include "pidman.h"

int main(int argc, char const *argv[]) {

    allocate_map();

    int pid;
    while ((pid = allocate_pid()) != 1)
        printf("pid allocato --> %d\n", pid);

    printf("pid tutti occupati\n"
           "liberiamone qualcuno\n");

    release_pid(300);
    release_pid(350);
    release_pid(400);
    release_pid(4700);
    release_pid(4800);
    release_pid(4999);
    release_pid(5000);

    while ((pid = allocate_pid()) != 1)
        printf("pid allocato --> %d\n", pid);


}
