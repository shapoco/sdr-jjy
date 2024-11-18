#pragma once

#define USE_FXP_LIB // todo 削除

#include <stdint.h>
#include <string.h>
#include <math.h>

#include "shapoco/fixed12.hpp"
#include "shapoco/jjy/common.hpp"
#include "shapoco/jjy/rx/common.hpp"

namespace shapoco::jjy::rx {

using namespace ::shapoco;

class DifferentialDetector {
public:
    freq_t freq = freq_t::EAST_40KHZ;
    
public:
    void init(freq_t freq, uint32_t t_now_ms) {
        this->freq = freq;
    }

    int32_t process(uint32_t t_now_ms, const int32_t *in) {
        int32_t sum = 0;
        int32_t delay = (freq == freq_t::EAST_40KHZ) ? DETECTION_RESOLUTION * 3 / 2 : DETECTION_RESOLUTION;
        for (int i = 0; i < (int)DETECTION_BLOCK_SIZE; i++) {
            int32_t sample0 = in[i];
            int32_t sample1 = in [(i + delay) % DETECTION_BLOCK_SIZE];
            sum += JJY_ABS(sample0 + sample1) / 2;
        }
        sum /= DETECTION_BLOCK_SIZE;
        return sum;
    }
};

}
