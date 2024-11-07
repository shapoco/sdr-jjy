#ifndef JJY_COMMON_HPP
#define JJY_COMMON_HPP

#ifndef SHAPOCO_JJY_CFG_RX_ADC_SPS
#define SHAPOCO_JJY_CFG_RX_ADC_SPS (480 * 1000)
#endif

#include <stdint.h>

namespace jjy {

static constexpr int PREC = 12; // 演算精度
static constexpr int32_t ONE = 1 << PREC;
static constexpr int32_t PHASE_PERIOD = ONE;

namespace rx {
    static constexpr uint32_t FREQ_60KHZ = 60 * 1000;
    static constexpr uint32_t DETECTION_INPUT_SPS = SHAPOCO_JJY_CFG_RX_ADC_SPS;   
    static constexpr uint32_t DETECTION_RESOLUTION = SHAPOCO_JJY_CFG_RX_ADC_SPS / FREQ_60KHZ;

    static constexpr uint32_t DETECTION_OUTPUT_SPS = 100;
    static constexpr uint32_t DETECTION_BLOCK_SIZE = DETECTION_INPUT_SPS / DETECTION_OUTPUT_SPS;
    static constexpr uint32_t PREFERRED_DMA_SIZE = DETECTION_BLOCK_SIZE;

    static_assert(DETECTION_RESOLUTION * FREQ_60KHZ == DETECTION_INPUT_SPS);
    static_assert(DETECTION_BLOCK_SIZE * DETECTION_OUTPUT_SPS == DETECTION_INPUT_SPS);
}

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

static int32_t phase_add(int32_t x, int32_t delta) {
    int32_t ret = x + delta;
    if (ret < 0) {
        do { ret += PHASE_PERIOD; } while (ret < 0);
    }
    else if (ret >= PHASE_PERIOD) {
        do { ret -= PHASE_PERIOD; } while (ret >= PHASE_PERIOD);
    }
    return ret;
}

static int32_t phase_diff(int32_t a, int32_t b) {
    int32_t diff = a - b;
    if (diff < -PHASE_PERIOD / 2) {
        do { diff += PHASE_PERIOD; } while (diff < -PHASE_PERIOD / 2);
    }
    else if (diff >= PHASE_PERIOD / 2) {
        do { diff -= PHASE_PERIOD; } while (diff >= PHASE_PERIOD / 2);
    }
    return diff;
}

static void phase_follow(int32_t *x, int32_t goal, int32_t ratio) {
    int32_t diff = phase_diff(goal, *x);
    if (diff == 0) return;
    int32_t step = diff * ratio / ONE;
    if (step == 0) {
        step = diff >= 0 ? 1 : -1;
    }
    *x = phase_add(*x, step);
}

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
