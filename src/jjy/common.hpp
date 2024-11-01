#ifndef JJY_COMMON_HPP
#define JJY_COMMON_HPP

#include <stdint.h>

namespace jjy {

static constexpr int PREC = 12; // 演算精度

typedef enum {
    WEST_60KHZ,
    EAST_40KHZ,
} freq_t;

#define JJY_ABS(x) ((x) < 0 ? -(x) : (x))
#define JJY_MIN(a, b) ((a) < (b) ? (a) : (b))
#define JJY_MAX(a, b) ((a) > (b) ? (a) : (b))
#define JJY_CLIP(min, max, val) (JJY_MAX((min), JJY_MIN((max), (val))))

template<typename T>
static constexpr T gcd(const T a, const T b) { return (b == 0) ? a : gcd(b, a % b); }

template<typename T>
static constexpr T lcm(const T a, const T b) { return a / gcd(a, b) * b; }

// 平方根を速く求める
// http://senta.s112.xrea.com/senta/Tips/000/c6/index.html
static inline uint32_t fast_sqrt(const uint32_t x) {
    if (x == 0) return 0;
    if (x == 1) return 1;
    
    int ret;
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

#endif
