#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <nucleus.h>
#include <libuarm.h>

static inline uint64_t pack(const uint32_t RegHigh, const uint32_t RegLow) {
    uint64_t rval = RegHigh;
    return (rval << 32) | RegLow;
}

/* Unpacks Reg in RegLow and RegHigh*/
static inline unpack(const uint64_t Reg, uint32_t *RegHigh, uint32_t *RegLow) {
    *RegLow = (uint32_t) Reg;
    *RegHigh = (uint32_t) (Reg >> 32);
}

typedef struct {
    uint64_t starting_point;
    uint64_t total;
} stopwatch_t;


#define STOPWATCH(name) \
	stopwatch_t name = {0, 0}


static inline void stopwatch_init(stopwatch_t *stopwatch)
{
    stopwatch->starting_point = stopwatch->total = 0;
}

// TODHI non farà mai overflow
static inline void stopwatch_start(stopwatch_t *stopwatch)
{
    stopwatch->starting_point = pack((uint32_t) getTODHI(), (uint32_t) getTODLO());
}

static inline void stopwatch_stop(stopwatch_t *stopwatch)
{
    uint64_t now = pack((uint32_t) getTODHI(), (uint32_t) getTODLO());
    stopwatch->total += now - stopwatch->starting_point;
}

static inline cputime stopwatch_get(stopwatch_t *stopwatch)
{
    return stopwatch->total;
}

static inline void stopwatch_reset(stopwatch_t *stopwatch)
{
    stopwatch_init(stopwatch);
}


#endif
