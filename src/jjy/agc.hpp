#ifndef SHAPOCO_JJY_AGC_HPP
#define SHAPOCO_JJY_AGC_HPP

#include <stdint.h>

#include "jjy/common.hpp"
#include "shapoco/math_utils.hpp"
#include "shapoco/ring_scope.hpp"

using namespace shapoco;

namespace jjy {

template<
    int PHASE_PERIOD, 
    int PHASE_RESOLUTION, 
    int32_t INIT_VAL = 0, 
    int32_t MIN_GAIN = ONE / 100, 
    int32_t MAX_GAIN = ONE * 100, 
    int32_t INIT_GAIN = MIN_GAIN
>
class Agc {
public:
    int32_t gain = INIT_GAIN;
    int32_t amplitude_peak = 0;
    int32_t out = INIT_VAL;

private:
    int phase = 0;
    RingScope<int32_t, int, INIT_VAL, PHASE_PERIOD, PHASE_RESOLUTION> history;

public:
    void reset() {
        phase = 0;
        history.clear(phase, INIT_VAL);
        gain = INIT_GAIN;
        amplitude_peak = 0;
        out = INIT_VAL;
    }
    
    int32_t process(int32_t in) {
        history.write(phase, SHPC_ABS(in));
        phase = (phase + 1 < PHASE_PERIOD) ? (phase + 1) : 0;

        amplitude_peak = history.max(INIT_VAL, true);
        if (amplitude_peak > 0) {
            gain = SHPC_ROUND_DIV(ONE * ONE, amplitude_peak);
        }
        else {
            gain = MAX_GAIN;
        }
        return (out = in * gain);
    }
};

}

#endif
