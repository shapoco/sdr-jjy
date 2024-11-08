#pragma once

#include <stdint.h>
#include <math.h>

#include "shapoco/jjy/common.hpp"
#include "shapoco/jjy/rx/common.hpp"

namespace shapoco::jjy::rx {

class QuadDetector {
public:

    static constexpr int RESO_60KHZ = DETECTION_RESOLUTION;
    static constexpr int RESO_40KHZ = DETECTION_RESOLUTION * 3 / 2;
    static constexpr int PERIOD = lcm(RESO_60KHZ, RESO_40KHZ);

    freq_t freq = freq_t::EAST_40KHZ;

private:
    int32_t sin_table_40kHz[PERIOD];
    int32_t sin_table_60kHz[PERIOD];
    int phase;
    
public:
    QuadDetector() {
        // sinテーブル
        for (int i = 0; i < PERIOD; i++) {
            sin_table_40kHz[i] = round(sin(i * 2 * M_PI / RESO_40KHZ) * (1 << (PREC - 1)));
        }
        for (int i = 0; i < PERIOD; i++) {
            sin_table_60kHz[i] = round(sin(i * 2 * M_PI / RESO_60KHZ) * (1 << (PREC - 1)));
        }
    }

    void init(freq_t freq, uint32_t t_now_ms) {
        this->freq = freq;
        phase = 0;
    }

    int32_t process(uint32_t t_now_ms, const int32_t *in) {
        int32_t sum = 0;
        for (int i = 0; i < DETECTION_BLOCK_SIZE; i++) {
            int32_t sin, cos;
            if (freq == freq_t::EAST_40KHZ) {
                sin = sin_table_40kHz[phase];
                cos = sin_table_40kHz[(phase + RESO_40KHZ / 4) % RESO_40KHZ];
            }
            else {
                sin = sin_table_60kHz[phase];
                cos = sin_table_60kHz[(phase + RESO_60KHZ / 4) % RESO_60KHZ];
            }
            int32_t u = (sin * in[i]) >> PREC;
            int32_t v = (cos * in[i]) >> PREC;
            sum += (uint32_t)fast_sqrt(u * u + v * v);
            phase = (phase + 1) % DETECTION_RESOLUTION;
        }
        return sum / DETECTION_BLOCK_SIZE;
    }
};

}
