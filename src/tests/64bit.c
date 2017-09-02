#include <stdio.h>
#include <stdint.h>


static inline uint64_t pack(const uint32_t RegLow, const uint32_t RegHigh) {
    uint64_t rval = RegHigh;
    return (rval << 32) | RegLow;
}

/* Unpacks Reg in RegLow and RegHigh*/
static inline void unpack(const uint64_t Reg, uint32_t *RegLow, uint32_t *RegHigh) {
    *RegLow = (uint32_t) Reg;
    *RegHigh = (uint32_t) (Reg >> 32);
}

int main(int argc, char const *argv[]) {
    uint32_t Low = 255;
    uint32_t High = 255;
    uint64_t full = pack(Low, High);
    printf("%lx\n", full);

    unpack(full, &Low, &High);
    printf("Low == %d, High == %d\n", Low, High);
    return 0;
}
