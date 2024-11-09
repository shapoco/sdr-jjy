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
    static constexpr int SCOPE_RESOLUTION = 8;
    static constexpr int SCOPE_NUM_PHASES = 4;
    static constexpr int SCOPE_SIZE = SCOPE_RESOLUTION * SCOPE_NUM_PHASES;

    freq_t freq = freq_t::EAST_40KHZ;

private:
    uint32_t scope[SCOPE_SIZE];
    
public:
    void init(freq_t freq, uint32_t t_now_ms) {
        this->freq = freq;
        memset(scope, 0, sizeof(scope));
    }

    int32_t process(uint32_t t_now_ms, const int32_t *in) {
        memset(scope, 0, sizeof(scope));
        
        int32_t sum = 0;
        int32_t delay = (freq == freq_t::EAST_40KHZ) ? DETECTION_RESOLUTION * 3 / 2 : DETECTION_RESOLUTION;
        for (int i = 0; i < DETECTION_BLOCK_SIZE; i++) {
            int32_t sample_c = in[i];
            int32_t sample_d = in [(i + delay) % DETECTION_BLOCK_SIZE];
            int32_t mul = JJY_ABS(sample_c + sample_d) / 2;
            sum += mul;

            if (i < SCOPE_RESOLUTION * SCOPE_NUM_PHASES * 3) {
                int scope_val = FXP_CLIP(0, 31, JJY_ROUND_DIV(((sample_c / 2) + ONE) * 31, ONE * 2));
                scope[i & (SCOPE_SIZE - 1)] |= (1 << scope_val);
            }
        }
        sum /= DETECTION_BLOCK_SIZE;
        return sum;
    }

    void readScope(uint32_t *buff) {
        memcpy(buff, scope, sizeof(scope));
    }
};

}
