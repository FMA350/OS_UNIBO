#include "pidman.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>

#define THREADS 10000

void *runner(void *);


int main(int argc, char const *argv[]) {
    allocate_map();
    pthread_t tid[THREADS];
    for (size_t i = 0; i < THREADS; i++) {
        pthread_create(&tid[i], NULL, runner, NULL);
    }
    for (size_t i = 0; i < THREADS; i++) {
        pthread_join(tid[i], NULL);
    }

    return 0;
}

void *runner(void *arg){
    int pid = allocate_pid();
    if (pid == 1) {
        fprintf(stderr, " unable to allocate a pid\n");
        pthread_exit((void *)(uintptr_t) 1);
    }
    printf("pid - %d\n", pid);
    srand(clock());
    printf("%d - starting to sleep...\n", pid);
    sleep(rand()%5+1);
    printf("%d - relasing the pid...\n", pid);
    release_pid(pid);

    pthread_exit((void *)(uintptr_t) 0);
}
