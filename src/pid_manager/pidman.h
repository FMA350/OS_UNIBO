#ifndef PIDMAN_H
#define PIDMAN_H

/* Creates and initializes a data structure for representing pids */
void allocate_map(void);

/*
 * Allocates and returns a pid;
 *
 * Postconditions:
 *
 * returns 1 if unable to allocate a pid (all pids are in use),
 * pid otherwise
 */

int allocate_pid(void);

/*
 * Releases a pid
 *
 * Preconditions: pid should be a used value
 */
void release_pid(int pid);

#endif
