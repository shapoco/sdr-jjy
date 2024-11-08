#ifndef SHAPOCO_RING_HISTORY_HPP
#define SHAPOCO_RING_HISTORY_HPP

#include <stdint.h>

namespace shapoco {

template<typename T, int PERIOD>
class RingHistory {
private:
    T _history[PERIOD];
    int _cursor = 0;
    int _size = 0;

public:
    void clear(T value = 0, bool fill = false) {
        for (int i = 0; i < PERIOD; i++) {
            _history[i] = value;
        }
        _cursor = 0;
        _size = fill ? PERIOD : 0;
    }

    bool push(T value) {
        _history[_cursor] = value;
        if (_size < PERIOD) {
            _size += 1;
        }
        if (_cursor < PERIOD - 1) {
            _cursor += 1;
            return false;
        }
        else {
            _cursor = 0;
            return true;
        }
    }

    int size() const { return _size; }

    T sum() const {
        T accum = 0;
        for (int i = 0; i < _size; i++) {
            accum += _history[i];
        }
        return accum;
    }

    T ave(T defaultValue = 0) const { 
        if (_size == 0) return defaultValue;
        return (_size > 0) ? (sum() / _size) : 0;
    }

    T min(T defaultValue = 0) const {
        if (_size == 0) return defaultValue;
        T min = 0;
        for (int i = 0; i < _size; i++) {
            T val = _history[i];
            if (i == 0) min = val;
            else if (val < min) min = val;
        }
        return min;
    }

    T max(T defaultValue = 0) const {
        if (_size == 0) return defaultValue;
        T max = 0;
        for (int i = 0; i < _size; i++) {
            T val = _history[i];
            if (i == 0) max = val;
            else if (val > max) max = val;
        }
        return max;
    }

};

}

#endif
