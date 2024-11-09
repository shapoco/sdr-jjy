#pragma once

#include <stdint.h>

#include "shapoco/fixed12.hpp"
#include "shapoco/jjy/common.hpp"
#include "shapoco/math_utils.hpp"
#include "shapoco/ring_history.hpp"

namespace shapoco::jjy {

template<
    int PERIOD, 
    int32_t INIT_BIAS = 0,
    int32_t SMOOTHING_RATIO = (fxp12::ONE >> 6)
>
class DcBias {
public:
    int32_t curr_bias = INIT_BIAS;
    int32_t peak_hi = 0;
    int32_t peak_lo = 0;
    int32_t out = 0;

private:
    RingHistory<int32_t, PERIOD> history;
    int32_t goal_bias = INIT_BIAS;

public:
    void reset() {
        history.clear(INIT_BIAS);
        peak_hi = INIT_BIAS;
        peak_lo = INIT_BIAS;
        goal_bias = INIT_BIAS;
        curr_bias = INIT_BIAS;
        out = 0;
    }
    
    int32_t process(int32_t in) {
        history.push(in);
        peak_hi = history.max();
        peak_lo = history.min();
        goal_bias = SHPC_ROUND_DIV(peak_hi + peak_lo, 2);
        
        curr_bias = fxp12::follow(curr_bias, goal_bias, SMOOTHING_RATIO);

        return (out = in - curr_bias);
    }
};

}
