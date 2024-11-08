#ifndef SHAPOCO_JJY_HYSTERESIS_HPP
#define SHAPOCO_JJY_HYSTERESIS_HPP

#include <stdint.h>

#include "shapoco/jjy/common.hpp"
#include "shapoco/ring_history.hpp"

using namespace shapoco;

namespace jjy {

template<int DEPTH, int HISTORY_SIZE, int32_t INIT_VAL = 0, int32_t HYST_RATIO = ONE / 10>
class Hysteresis {
public:
    int32_t peak_hi = INIT_VAL;
    int32_t peak_lo = INIT_VAL;
    int32_t threshold = INIT_VAL;
    int32_t hysteresis = 0;
    int32_t out_anl = INIT_VAL;
    uint8_t out_dig = 0;

private:
    RingHistory<int32_t, HISTORY_SIZE> history;

public:
    void reset() {
        history.clear(INIT_VAL);
        peak_hi = INIT_VAL;
        peak_lo = INIT_VAL;
        threshold = INIT_VAL;
        hysteresis = 0;
        out_anl = INIT_VAL;
        out_dig = 0;
    }
    
    uint8_t process(uint8_t in) {
        history.push(in);

        out_anl = in + (out_dig ? -hysteresis : hysteresis);
        out_dig = out_anl >= threshold;

        peak_hi = history.max();
        peak_lo = history.min();
        threshold = (peak_hi + peak_lo) / 2;
        hysteresis = (peak_hi - peak_lo) * HYST_RATIO / ONE;

        return out_dig;
    }
};

}

#endif
