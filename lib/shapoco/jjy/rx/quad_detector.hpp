#pragma once

#include <stdint.h>
#include <string.h>
#include <math.h>

#include "shapoco/fixed12.hpp"
#include "shapoco/jjy/common.hpp"
#include "shapoco/jjy/rx/common.hpp"

namespace shapoco::jjy::rx {

using namespace ::shapoco;

class QuadDetector {
public:
    static constexpr int SIN_TABLE_SIZE = DETECTION_RESOLUTION * 3;

    int32_t sinTable40kHz[SIN_TABLE_SIZE * 2];
    int32_t sinTable60kHz[SIN_TABLE_SIZE * 2];
    int theta = 0;

    QuadDetector() { 
        for (int i = 0; i < SIN_TABLE_SIZE; i++) {
            sinTable40kHz[i] = fxp12::fastSin(i * fxp12::PHASE_PERIOD * 2 / SIN_TABLE_SIZE);
            sinTable60kHz[i] = fxp12::fastSin(i * fxp12::PHASE_PERIOD * 3 / SIN_TABLE_SIZE);
            //sinTable40kHz[i] = ::sin(M_PI * 2 * i * 2 / SIN_TABLE_SIZE) * fxp12::ONE;
            //sinTable60kHz[i] = ::sin(M_PI * 2 * i * 3 / SIN_TABLE_SIZE) * fxp12::ONE;
            sinTable40kHz[i + SIN_TABLE_SIZE] = sinTable40kHz[i];
            sinTable60kHz[i + SIN_TABLE_SIZE] = sinTable60kHz[i];
        }
    }

    void init(uint64_t nowMs) {
        theta = 0;
    }

    int32_t process(uint64_t nowMs, const int32_t *in) {
        constexpr int WAVE_LEN_40KHZ = DETECTION_RESOLUTION * 3 / 2;
        constexpr int WAVE_LEN_60KHZ = DETECTION_RESOLUTION;
        constexpr int HALF_WAVE_LEN_40KHZ = WAVE_LEN_40KHZ / 2;
        constexpr int HALF_WAVE_LEN_60KHZ = WAVE_LEN_60KHZ / 2;
        constexpr int SUM_ARRAY_LEN = SHPC_MAX(HALF_WAVE_LEN_40KHZ, HALF_WAVE_LEN_60KHZ);
        int32_t sum40kHz[SUM_ARRAY_LEN];
        int32_t sum60kHz[SUM_ARRAY_LEN];
        memset(sum40kHz, 0, sizeof(sum40kHz));
        memset(sum60kHz, 0, sizeof(sum60kHz));
        for (int i = 0; i < (int)DETECTION_BLOCK_SIZE; i++) {
            int32_t sample = in[i];
            int a = theta;
            for (int phase = 0; phase < SUM_ARRAY_LEN; phase++) {
                sum40kHz[phase] += fxp12::roundToInt(sinTable40kHz[a] * sample);
                sum60kHz[phase] += fxp12::roundToInt(sinTable60kHz[a] * sample);
                a += 1;
            }   
            theta = SHPC_CYCLIC_INCR(theta, SIN_TABLE_SIZE);
        }
        int32_t out40kHz = 0;
        int32_t out60kHz = 0;
        for (int phase = 0; phase < HALF_WAVE_LEN_40KHZ; phase++) {
            out40kHz = SHPC_MAX(out40kHz, shapoco::abs(SHPC_ROUND_DIV(sum40kHz[phase], (int)DETECTION_BLOCK_SIZE)));
        }
        for (int phase = 0; phase < HALF_WAVE_LEN_60KHZ; phase++) {
            out60kHz = SHPC_MAX(out60kHz, shapoco::abs(SHPC_ROUND_DIV(sum60kHz[phase], (int)DETECTION_BLOCK_SIZE)));
        }
        //return /*out40kHz +*/ out60kHz;
        return out60kHz;
    }
};

}
