#pragma once

#include <stdint.h>

#define SHPC_INLINE inline __attribute__((always_inline))
#define SHPC_STATIC_INLINE static SHPC_INLINE

#define SHPC_ABS(x) ((x) >= 0 ? (x) : -(x))
#define SHPC_MIN(a, b) ((a) < (b) ? (a) : (b))
#define SHPC_MAX(a, b) ((a) > (b) ? (a) : (b))
#define SHPC_CLIP(min, max, val) (SHPC_MAX((min), SHPC_MIN((max), (val))))

#define SHPC_CEIL_DIV(a, b) (((a) + (b) - 1) / (b))
#define SHPC_ROUND_DIV(a, b) (((a) + ((b) / 2)) / (b))

#define SHPC_CYCLIC_INCR(x, period) (((x) + 1 < (period)) ? ((x) + 1) : 0)
#define SHPC_CYCLIC_DECR(x, period) (((x) - 1 >= 0) ? ((x) - 1) : (period) - 1)

namespace shapoco {

SHPC_STATIC_INLINE int32_t cyclicNorm(int32_t x, int32_t period) {
    if (x < 0) {
        do { x += period; } while (x < 0);
    }
    else if (x >= period) {
        do { x -= period; } while (x >= period);
    }
    return x;
}

SHPC_STATIC_INLINE int32_t cyclicAdd(int32_t a, int32_t b, int32_t period) {
    return cyclicNorm(a + b, period);
}

SHPC_STATIC_INLINE int32_t cyclicDiff(int32_t a, int32_t b, int32_t period) {
    return cyclicNorm(a - b + (period / 2), period) - (period / 2);
}

SHPC_STATIC_INLINE int32_t cyclicFollow(int32_t x, int32_t goal, int32_t period, int32_t ratioFxp, int fxpPrec, int32_t maxSpeed = -1) {
    int32_t diff = cyclicDiff(goal, x, period);
    if (diff == 0) return x;
    int32_t step = SHPC_ROUND_DIV(diff * ratioFxp, 1 << fxpPrec);
    if (step == 0) {
        step = diff >= 0 ? 1 : -1;
    }
    else if (maxSpeed > 0) {
        step = SHPC_CLIP(-maxSpeed, maxSpeed, step);
    }
    return cyclicAdd(x, step, period);
}

}
