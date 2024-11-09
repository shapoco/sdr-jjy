#pragma once

#include <stdint.h>

#define FXP_INLINE inline __attribute__((always_inline))

#define FXP_ABS(x) ((x) >= 0 ? (x) : -(x))
#define FXP_MIN(a, b) ((a) < (b) ? (a) : (b))
#define FXP_MAX(a, b) ((a) > (b) ? (a) : (b))
#define FXP_CLIP(min, max, val) (FXP_MAX((min), FXP_MIN((max), (val))))

namespace shapoco::fxp12 {

constexpr int PREC = 12;
constexpr int32_t ONE = 1 << PREC;

constexpr int32_t PHASE_PREC = 12;
constexpr int32_t PHASE_PERIOD = 1 << PHASE_PREC;

extern int32_t sin_table[PHASE_PERIOD];

void init_tables(void);
int32_t normalize_angle(int32_t a);
int32_t fast_sin(int32_t a);
int32_t fast_cos(int32_t a);
int32_t log2(int32_t x);

static FXP_INLINE int to_int(int32_t x) { return x >> PREC; }
static FXP_INLINE int round_to_int(int32_t x) { return to_int(x + (ONE / 2)); }

static FXP_INLINE int32_t follow(int32_t val, int32_t goal, int32_t ratio) {
    int32_t diff = goal - val;
    if (diff == 0) return val;
    int32_t step = diff * ratio / ONE;
    if (step == 0) step = diff >= 0 ? 1 : -1;
    return val + step;
}

static FXP_INLINE int32_t phase_norm(int32_t x, int32_t offset = 0) {
    return (x + offset) & (PHASE_PERIOD - 1);
}

static FXP_INLINE int32_t phase_add(int32_t x, int32_t delta) {
    return phase_norm(x + delta);
}

static FXP_INLINE int32_t phase_diff(int32_t a, int32_t b) {
    return phase_norm(a - b, -PHASE_PERIOD / 2);
}

static FXP_INLINE void phase_follow(int32_t *x, int32_t goal, int32_t ratio) {
    int32_t diff = phase_diff(goal, *x);
    if (diff == 0) return;
    int32_t step = diff * ratio / ONE;
    if (step == 0) {
        step = diff >= 0 ? 1 : -1;
    }
    *x = phase_add(*x, step);
}

}
