#pragma once

#include <stdint.h>

namespace shapoco::jjy {

static constexpr int PREC = 12; // 演算精度
static constexpr int32_t ONE = 1 << PREC;
static constexpr int32_t PHASE_PERIOD = ONE;

typedef enum {
    WEST_60KHZ,
    EAST_40KHZ,
} freq_t;

enum class jjybit_t : uint8_t {
    ZERO = 0,
    ONE = 1,
    MARKER = 2,
    ERROR = 3,
};

#define JJY_STATIC_INLINE static inline __attribute__((always_inline))

#define JJY_ABS(x) ((x) >= 0 ? (x) : -(x))
#define JJY_MIN(a, b) ((a) < (b) ? (a) : (b))
#define JJY_MAX(a, b) ((a) > (b) ? (a) : (b))
#define JJY_CLIP(min, max, val) (JJY_MAX((min), JJY_MIN((max), (val))))

#define JJY_CEIL_DIV(a, b) (((a) + (b) - 1) / (b))
#define JJY_ROUND_DIV(a, b) (((a) + (b) / 2) / (b))

template<typename T>
static constexpr T gcd(const T a, const T b) { return (b == 0) ? a : gcd(b, a % b); }

template<typename T>
static constexpr T lcm(const T a, const T b) { return a / gcd(a, b) * b; }

static int32_t phaseAdd(int32_t x, int32_t delta) {
    int32_t ret = x + delta;
    if (ret < 0) {
        do { ret += PHASE_PERIOD; } while (ret < 0);
    }
    else if (ret >= PHASE_PERIOD) {
        do { ret -= PHASE_PERIOD; } while (ret >= PHASE_PERIOD);
    }
    return ret;
}

static int32_t phaseDiff(int32_t a, int32_t b) {
    int32_t diff = a - b;
    if (diff < -PHASE_PERIOD / 2) {
        do { diff += PHASE_PERIOD; } while (diff < -PHASE_PERIOD / 2);
    }
    else if (diff >= PHASE_PERIOD / 2) {
        do { diff -= PHASE_PERIOD; } while (diff >= PHASE_PERIOD / 2);
    }
    return diff;
}

static void phaseFollow(int32_t *x, int32_t goal, int32_t ratio) {
    int32_t diff = phaseDiff(goal, *x);
    if (diff == 0) return;
    int32_t step = diff * ratio / ONE;
    if (step == 0) {
        step = diff >= 0 ? 1 : -1;
    }
    *x = phaseAdd(*x, step);
}

// 平方根を速く求める
// http://senta.s112.xrea.com/senta/Tips/000/c6/index.html
JJY_STATIC_INLINE int32_t fast_sqrt(const int32_t x) {
    if (x <= 1) return x;

    int32_t ret;
    if (x <= 38408) ret = (x >> 7) + 11;
    else if (x <= 1411319) ret = (x >> 10) + 210;
    else if (x <= 70459124) ret = (x >> 13) + 1414;
    else if (x <= 794112116) ret = (x >> 15) + 7863;
    else ret = (x >> 17) + 26038;

    ret = (ret + x / ret) >> 1;
    ret = (ret + x / ret) >> 1;
    return ret;
}

}
