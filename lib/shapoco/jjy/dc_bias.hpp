#pragma once

#include <stdint.h>

#include "shapoco/jjy/common.hpp"
#include "shapoco/math_utils.hpp"
#include "shapoco/ring_history.hpp"

namespace shapoco::jjy {

template<int PERIOD, int32_t INIT_BIAS = 0>
class DcBias {
public:
    int32_t bias = INIT_BIAS;
    int32_t peak_hi = 0;
    int32_t peak_lo = 0;
    int32_t out = 0;

private:
    RingHistory<int32_t, PERIOD> history;

public:
    void reset() {
        history.clear(INIT_BIAS);
        peak_hi = INIT_BIAS;
        peak_lo = INIT_BIAS;
        bias = INIT_BIAS;
        out = 0;
    }
    
    int32_t process(int32_t in) {
        history.push(in);
        peak_hi = history.max();
        peak_lo = history.min();
        bias = SHPC_ROUND_DIV(peak_hi + peak_lo, 2);
        return (out = in - bias);
    }
};

}
