#include "pidman.h"

#define MIN_PID 300
#define MAX_PID 5000

/* number of pids available */
/* Used as size */

#define PIDS    \
    (MAX_PID - MIN_PID + 1)

/* Used in operations */
static const int pids = PIDS;

/* MIN_PID <= PID <= MAX_PID */
#define INDEX_OF(PID)   \
    (PID - MIN_PID)

/* INDEX < pids */
#define PID_OF(INDEX)   \
    (INDEX + MIN_PID)

/*
 * pid_bitmap[i] == 0   ==> the pid is free
 * pid_bitmap[i] == 1   ==> the process is currently in use
 */
static int pid_bitmap[PIDS];

/* keeps track of the next possible free pid */
static int iter = 0;

#define NEXT_ITER(ITER)  \
    ((ITER + 1) % pids)

#define LAST_ITER(ITER)  \
    (ITER == 0 ? pids - 1: ITER - 1)

void inline allocate_map(void){
    for (int i = 0; i < pids; i++)
        pid_bitmap[i] = 0;
}

int allocate_pid(void){
    int found = 0;
    /* counts the number of looked up element so far */
    int counter = 0;

    while (!found && counter < pids) {
        if (pid_bitmap[iter] == 0){
            found = 1;
            pid_bitmap[iter] = 1;
        }
        counter++;
        iter = NEXT_ITER(iter);
    }

    if (found)
        return(PID_OF(LAST_ITER(iter)));
    else
        return 1;
}

void inline release_pid(int pid){
    pid_bitmap[INDEX_OF(pid)] = 0;
}
