#pragma once

#include <stdint.h>

#include "shapoco/math_utils.hpp"

namespace shapoco {

template<typename T, T DEFAULT_VALUE = 0>
class PeakHold {
private:
    bool _empty;
    T _base, _peak;

public:
    PeakHold() : _empty(true), _base(DEFAULT_VALUE), _peak(DEFAULT_VALUE) { }

    void clear(T value = DEFAULT_VALUE) {
        _base = _peak = value;
        _empty = true;
    }

    void enter(T value) {
        if (_empty) {
            _base = _peak = value;
            _empty = false;
        }
        else {
            _base = SHPC_MIN(_base, value);
            _peak = SHPC_MAX(_peak, value);
        }
    }

    T base() const { return _base; }
    T peak() const { return _peak; }
    T amplitude() const { return _peak - _base; }
};

}
