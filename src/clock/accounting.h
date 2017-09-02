/*                               _              _
                                | |            | |
  __ _  ___ ___ ___  _   _ _ __ | |_ __ _ _ __ | |_
 / _` |/ __/ __/ _ \| | | | '_ \| __/ _` | '_ \| __|
| (_| | (_| (_| (_) | |_| | | | | || (_| | | | | |_
\__,_|\___\___\___/ \__,_|_| |_|\__\__,_|_| |_|\__|


*/
#ifndef ACCOUNTING_H
#define ACCOUNTING_H

// defined in accounting.c
extern unsigned int timeSliceLeft;
extern unsigned int clockPerTimeslice;

unsigned int accountant(struct tcb_t* thread);
void update_clock(unsigned int milliseconds);


static inline uint64_t pack(const uint32_t RegLow, const uint32_t RegHigh) {
    uint64_t rval = RegHigh;
    return (rval << 32) | RegLow;
}

/* Unpacks Reg in RegLow and RegHigh*/
static inline unpack(const uint64_t Reg, uint32_t *RegLow, uint32_t *RegHigh) {
    *RegLow = (uint32_t) Reg;
    *RegHigh = (uint32_t) (Reg >> 32);
}

#endif
