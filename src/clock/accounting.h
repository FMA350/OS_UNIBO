/*                               _              _
                                | |            | |
  __ _  ___ ___ ___  _   _ _ __ | |_ __ _ _ __ | |_
 / _` |/ __/ __/ _ \| | | | '_ \| __/ _` | '_ \| __|
| (_| | (_| (_| (_) | |_| | | | | || (_| | | | | |_
\__,_|\___\___\___/ \__,_|_| |_|\__\__,_|_| |_|\__|


*/
#ifndef ACCOUNTING
#define ACCOUNTING

unsigned int accountant(struct tcb_t* thread);
void update_clock(unsigned int milliseconds);

#endif
