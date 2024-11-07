#ifndef RING_STAT_HPP
#define RING_STAT_HPP

#include <stdint.h>

template<typename T, int PERIOD>
class RingStat {
private:
    T _history[PERIOD];
    int _cursor = 0;
    int _size = 0;

public:
    void clear(T value) {
        for (int i = 0; i < PERIOD; i++) {
            _history[i] = value;
        }
        _cursor = 0;
        _size = 0;
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

    T sum(T initval = 0) const {
        T accum = initval;
        for (int i = 0; i < _size; i++) {
            accum += _history[i];
        }
        return accum;
    }

    T ave() const { 
        return (_size > 0) ? (sum() / _size) : 0;
    }

    T min() const {
        T min = 0;
        for (int i = 0; i < _size; i++) {
            T val = _history[i];
            if (i == 0) min = val;
            else if (val < min) min = val;
        }
        return min;
    }

    T max() const {
        T max = 0;
        for (int i = 0; i < _size; i++) {
            T val = _history[i];
            if (i == 0) max = val;
            else if (val > max) max = val;
        }
        return max;
    }

};

#endif
