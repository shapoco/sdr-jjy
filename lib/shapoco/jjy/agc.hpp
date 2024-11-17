#pragma once

#include <stdint.h>

#include "shapoco/fixed12.hpp"
#include "shapoco/jjy/common.hpp"
#include "shapoco/math_utils.hpp"
#include "shapoco/ring_history.hpp"

namespace shapoco::jjy {

template<
    int PERIOD, 
    int32_t INIT_VAL = 0, 
    int32_t MIN_GAIN = ONE / 100, 
    int32_t MAX_GAIN = ONE * 100, 
    int32_t INIT_GAIN = MIN_GAIN,
    int32_t SMOOTHING_RATIO = (fxp12::ONE >> 6)
>
class Agc {
public:
    int32_t curr_gain = INIT_GAIN;
    int32_t amplitude_peak = 0;
    int32_t out = INIT_VAL;

private:
    RingHistory<int32_t, PERIOD> history;
    int32_t goal_gain = INIT_GAIN;

public:
    void reset() {
        history.clear(INIT_VAL);
        curr_gain = INIT_GAIN;
        amplitude_peak = 0;
        out = INIT_VAL;
    }
    
    int32_t process(int32_t in) {
        history.push(shapoco::abs(in));

        amplitude_peak = history.max();
        if (amplitude_peak > 0) {
            goal_gain = SHPC_CLIP(MIN_GAIN, MAX_GAIN, SHPC_ROUND_DIV(ONE * ONE, amplitude_peak));
        }
        else {
            goal_gain = MAX_GAIN;
        }

        curr_gain = fxp12::follow(curr_gain, goal_gain, SMOOTHING_RATIO);

        return (out = in * curr_gain);
    }
};

}
