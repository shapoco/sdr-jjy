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

class QuadDetector {
public:
    static constexpr int SCOPE_RESOLUTION = 8;
    static constexpr int SCOPE_NUM_PHASES = 4;
    static constexpr int SCOPE_SIZE = SCOPE_RESOLUTION * SCOPE_NUM_PHASES;

    freq_t freq = freq_t::EAST_40KHZ;

private:
#ifdef USE_FXP_LIB
    static constexpr int32_t STEP_60KHZ = fxp12::PHASE_PERIOD * 60000 / DETECTION_INPUT_SPS;
    static constexpr int32_t STEP_40KHZ = fxp12::PHASE_PERIOD * 40000 / DETECTION_INPUT_SPS;
#else
    static constexpr int RESO_60KHZ = DETECTION_RESOLUTION;
    static constexpr int RESO_40KHZ = DETECTION_RESOLUTION * 3 / 2;
    static constexpr int TABLE_PERIOD = lcm(RESO_60KHZ, RESO_40KHZ);
    int32_t sin_table_40kHz[TABLE_PERIOD];
    int32_t sin_table_60kHz[TABLE_PERIOD];
#endif
    int phase_sin;

    uint32_t scope[SCOPE_SIZE];
    int scope_phase;
    
public:
#ifndef USE_FXP_LIB
    QuadDetector() {
        // sinテーブル
        for (int i = 0; i < TABLE_PERIOD; i++) {
            sin_table_40kHz[i] = round(sin(i * 2 * M_PI / RESO_40KHZ) * (1 << (PREC - 1)));
        }
        for (int i = 0; i < TABLE_PERIOD; i++) {
            sin_table_60kHz[i] = round(sin(i * 2 * M_PI / RESO_60KHZ) * (1 << (PREC - 1)));
        }
    }
#endif

    void init(freq_t freq, uint32_t t_now_ms) {
        this->freq = freq;
        phase_sin = 0;
        memset(scope, 0, sizeof(scope));
    }

    int32_t process(uint32_t t_now_ms, const int32_t *in) {
        memset(scope, 0, sizeof(scope));
        
        int32_t sum = 0;
        int32_t phase_step = freq == freq_t::EAST_40KHZ ? STEP_40KHZ : STEP_60KHZ;
        int32_t phase_cos = phase_sin + fxp12::PHASE_PERIOD / 4;
        for (int i = 0; i < DETECTION_BLOCK_SIZE; i++) {
            int32_t sample = in[i];

            int scope_val = FXP_CLIP(0, 31, JJY_ROUND_DIV(((sample / 2) + ONE) * 31, ONE * 2));
            scope_phase = (scope_phase + phase_step) & (fxp12::PHASE_PERIOD * SCOPE_NUM_PHASES - 1);
            scope[(scope_phase * SCOPE_RESOLUTION) >> fxp12::PHASE_PREC] |= (1 << scope_val);

#ifdef USE_FXP_LIB
            int32_t u = (fxp12::sin_table[phase_sin] * sample) >> PREC;
            int32_t v = (fxp12::sin_table[phase_cos] * sample) >> PREC;
            phase_sin = fxp12::phase_norm(phase_sin + phase_step);
            phase_cos = fxp12::phase_norm(phase_cos + phase_step);
#else
            if (freq == freq_t::EAST_40KHZ) {
                sin = sin_table_40kHz[phase];
                cos = sin_table_40kHz[(phase + RESO_40KHZ / 4) % RESO_40KHZ];
            }
            else {
                sin = sin_table_60kHz[phase];
                cos = sin_table_60kHz[(phase + RESO_60KHZ / 4) % RESO_60KHZ];
            }
            phase = (phase + 1) % DETECTION_RESOLUTION;
            int32_t u = (sin * sample) >> PREC;
            int32_t v = (cos * sample) >> PREC;
#endif
            sum += fast_sqrt(u * u + v * v);
        }
        return sum / DETECTION_BLOCK_SIZE;
    }

    void readScope(uint32_t *buff) {
        memcpy(buff, scope, sizeof(scope));
    }
};

}
