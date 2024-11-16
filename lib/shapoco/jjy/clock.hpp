#pragma once

#include <stdint.h>

#include "shapoco/jjy/common.hpp"
#include "shapoco/jjy/datetime.hpp"

namespace shapoco::jjy {

class JjyClock {
private:
    uint64_t lastMs;
    JjyDateTime lastDatetime;

public:
    void set(uint64_t nowMs, const JjyDateTime &dt) {
        lastMs = nowMs;
        lastDatetime = dt;
    }

    JjyDateTime get(uint64_t nowMs) {
        int64_t deltaMs = (int64_t)nowMs - (int64_t)lastMs;
        lastDatetime.addMillisSelf(deltaMs);
        lastMs = nowMs;
        return lastDatetime;
    }
};

}
