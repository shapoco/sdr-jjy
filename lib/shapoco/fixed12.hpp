#pragma once

#include <stdint.h>
#include "shapoco/math_utils.hpp"

// todo: SHPC_... に置き換え
#define FXP_ABS(x) ((x) >= 0 ? (x) : -(x))
#define FXP_MIN(a, b) ((a) < (b) ? (a) : (b))
#define FXP_MAX(a, b) ((a) > (b) ? (a) : (b))
#define FXP_CLIP(min, max, val) (FXP_MAX((min), FXP_MIN((max), (val))))

namespace shapoco::fxp12 {

constexpr int PREC = 12;
constexpr int32_t ONE = 1 << PREC;
constexpr int32_t FRAC_MASK = ONE - 1;

constexpr int32_t PHASE_PREC = 12;
constexpr int32_t PHASE_PERIOD = 1 << PHASE_PREC;

extern int32_t sinTable[PHASE_PERIOD];

void initTables(void);
int32_t normalize_angle(int32_t a);
int32_t fastSin(int32_t a);
int32_t fastCos(int32_t a);
int32_t log2(int32_t x);

SHPC_STATIC_INLINE int32_t floor(int32_t x) { return x & (~FRAC_MASK); }
SHPC_STATIC_INLINE int32_t ceil(int32_t x) { return fxp12::floor(x + ONE - 1); }
SHPC_STATIC_INLINE int32_t round(int32_t x) { return fxp12::floor(x + (ONE / 2)); }
SHPC_STATIC_INLINE int32_t trunc(int32_t x) { return shapoco::sign(x) * fxp12::floor(shapoco::abs(x)); }

SHPC_STATIC_INLINE int floorToInt(int32_t x) { return x >> PREC; }
SHPC_STATIC_INLINE int ceilToInt(int32_t x) { return fxp12::floorToInt(fxp12::ceil(x)); }
SHPC_STATIC_INLINE int roundToInt(int32_t x) { return fxp12::floorToInt(fxp12::round(x)); }
SHPC_STATIC_INLINE int truncToInt(int32_t x) { return fxp12::floorToInt(fxp12::trunc(x)); }

SHPC_STATIC_INLINE int32_t follow(int32_t val, int32_t goal, int32_t ratio) {
    int32_t diff = goal - val;
    if (diff == 0) return val;
    int32_t step = diff * ratio / ONE;
    if (step == 0) step = diff >= 0 ? 1 : -1;
    return val + step;
}

SHPC_STATIC_INLINE int32_t phaseNorm(int32_t x) {
    return cyclicNorm(x, PHASE_PERIOD);
}

SHPC_STATIC_INLINE int32_t phaseAdd(int32_t a, int32_t b) {
    return phaseNorm(a + b);
}

SHPC_STATIC_INLINE int32_t phaseDiff(int32_t a, int32_t b) {
    return phaseNorm(a - b + (PHASE_PERIOD / 2)) - (PHASE_PERIOD / 2);
}

// todo: 削除?
SHPC_STATIC_INLINE int32_t phaseFollow_tmp(int32_t x, int32_t goal, int32_t ratio, int32_t maxSpeed = -1) {
    int32_t diff = phaseDiff(goal, x);
    if (diff == 0) return x;
    int32_t step = diff * ratio / ONE;
    if (step == 0) {
        step = diff >= 0 ? 1 : -1;
    }
    else if (maxSpeed > 0) {
        step = FXP_CLIP(-maxSpeed, maxSpeed, step);
    }
    return phaseAdd(x, step);
}

}
