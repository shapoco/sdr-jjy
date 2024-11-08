#ifndef SHAPOCO_JJY_AGC_HPP
#define SHAPOCO_JJY_AGC_HPP

#include <stdint.h>

#include "jjy/common.hpp"
#include "shapoco/math_utils.hpp"
#include "shapoco/ring_history.hpp"

using namespace shapoco;

namespace jjy {

template<
    int PERIOD, 
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
    RingHistory<int32_t, PERIOD> history;

public:
    void reset() {
        history.clear(INIT_VAL);
        gain = INIT_GAIN;
        amplitude_peak = 0;
        out = INIT_VAL;
    }
    
    int32_t process(int32_t in) {
        history.push(SHPC_ABS(in));

        amplitude_peak = history.max();
        if (amplitude_peak > 0) {
            gain = SHPC_ROUND_DIV(ONE * ONE, amplitude_peak);
            gain = SHPC_CLIP(MIN_GAIN, MAX_GAIN, gain);
        }
        else {
            gain = MAX_GAIN;
        }
        return (out = in * gain);
    }
};

}

#endif
