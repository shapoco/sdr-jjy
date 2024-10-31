#ifndef JJY_COMMON_HPP
#define JJY_COMMON_HPP

#include <stdint.h>

namespace jjy {

static constexpr int PREC = 12; // 演算精度

#define JJY_ABS(x) ((x) < 0 ? -(x) : (x))
#define JJY_MIN(a, b) ((a) < (b) ? (a) : (b))
#define JJY_MAX(a, b) ((a) > (b) ? (a) : (b))
#define JJY_CLIP(min, max, val) (JJY_MAX((min), JJY_MIN((max), (val))))

template<typename T>
static inline T min(T a, T b) { return a < b ? a : b; }

template<typename T>
static inline T max(T a, T b) { return a > b ? a : b; }

template<typename T>
static inline T clip(T min, T max, T value) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

#if 0
// 高速根号計算 (fast sqrt algorithm)
// https://takashiijiri.com/study/miscs/fastsqrt.html
static inline float fast_sqrt(const float x) {
    float xHalf = 0.5f * x;
    int tmp = 0x5F3759DF - (*(int*)&x >> 1);
    float xRes = *(float*)&tmp;
    xRes *= (1.5f - (xHalf * xRes * xRes));
    return xRes * x;
}
#else
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
#endif

}

#endif
