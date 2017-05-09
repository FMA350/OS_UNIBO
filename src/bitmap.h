#include <stdint.h>   /* for uint32_t */

typedef uint32_t word_t;
enum { BITS_PER_WORD = sizeof(word_t) * 8 };

// 0 <= b < Max index
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)


static inline void set_bit(word_t *bitmap, unsigned int n) {
    bitmap[WORD_OFFSET(n)] |= (1 << BIT_OFFSET(n));
}

static inline void clear_bit(word_t *bitmap, unsigned int n) {
    bitmap[WORD_OFFSET(n)] &= ~(1 << BIT_OFFSET(n));
}

static inline int get_bit(word_t *bitmap, unsigned int n) {
    return ((bitmap[WORD_OFFSET(n)] & (1 << BIT_OFFSET(n))) != 0);
}
