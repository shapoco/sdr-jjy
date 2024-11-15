#pragma once

#include <stdint.h>

#include "shapoco/common.hpp"

namespace shapoco {

template<typename T, T PERIOD>
struct cyclic {

#define THIS_CYCLIC cyclic<T, PERIOD>

public:
    T raw;
    cyclic(T raw) : raw(cyclicNorm(raw, PERIOD)) {}

    THIS_CYCLIC operator+(T other) {
        return THIS_CYCLIC(cyclicAdd(raw, other, PERIOD));
    }

    T operator-(T other) {
        return cyclicDiff(raw, other, PERIOD);
    }

    T operator-(THIS_CYCLIC other) {
        return cyclicDiff(other.raw, other, PERIOD);
    }

    THIS_CYCLIC& operator++() {
        raw = SHPC_CYCLIC_INCR(raw, PERIOD);
        return *this;
    }
    
    THIS_CYCLIC operator++(int) {
        THIS_CYCLIC temp = *this;
        raw = SHPC_CYCLIC_INCR(raw, PERIOD);
        return temp;
    }

    THIS_CYCLIC& operator--() {
        raw = SHPC_CYCLIC_DECR(raw, PERIOD);
        return *this;
    }
    
    THIS_CYCLIC operator--(int) {
        THIS_CYCLIC temp = *this;
        raw = SHPC_CYCLIC_DECR(raw, PERIOD);
        return temp;
    }

    bool operator==(THIS_CYCLIC other) {
        return raw == other.raw;
    }

    operator T() const {
        return raw;
    }

};

#undef THIS_CYCLIC

}
