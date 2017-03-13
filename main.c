#include "mikabooq.h"



int main(int argc, char const *argv[]) {

    /* Initialization */
    struct pcb_t *root = proc_init();
    thread_init();
    msgq_init();

    /* go on */

    return 0;
}
