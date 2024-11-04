#ifndef FIXED12_HPP
#define FIXED12_HPP

#include <stdint.h>

namespace fxp12 {

#define FXP_INELINE inline __attribute__((always_inline))

#define FXP_ABS(x) ((x) >= 0 ? (x) : -(x))
#define FXP_MIN(a, b) ((a) < (b) ? (a) : (b))
#define FXP_MAX(a, b) ((a) > (b) ? (a) : (b))
#define FXP_CLIP(min, max, val) (FXP_MAX((min), FXP_MIN((max), (val))))

constexpr int PREC = 12;
constexpr int32_t ONE = 1 << PREC;
constexpr int32_t ANGLE_PERIOD = ONE;

void init_tables(void);
int32_t normalize_angle(int32_t a);
int32_t sin(int32_t a);
int32_t cos(int32_t a);
int32_t log2(int32_t x);

static FXP_INELINE int to_int(int32_t x) { return x >> PREC; }
static FXP_INELINE int round_to_int(int32_t x) { return to_int(x + (ONE / 2)); }

static FXP_INELINE void interp(int32_t *var, int32_t goal, int32_t ratio) {
    int32_t diff = goal - *var;
    if (diff == 0) return;
    int32_t step = diff * ratio / ONE;
    if (step == 0) step = diff >= 0 ? 1 : -1;
    *var += step;
}

}

#endif
