#pragma once

#include <stdint.h>

#include "shapoco/math_utils.hpp"

namespace shapoco {

template<typename TValue, typename TTime, TValue DEFAULT_VAL = 0, TTime PERIOD = 1000, TTime RESO = 1>
class RingScope {
public:
    static constexpr int HISTORY_SIZE = PERIOD / RESO;
    static_assert(HISTORY_SIZE * RESO == PERIOD);

private:
    typedef struct {
        bool valid;
        TValue min;
        TValue max;
        TValue ave;
    } ring_scope_entry_t;

    ring_scope_entry_t _history[HISTORY_SIZE];
    
    int _first_cursor = 0;
    int _last_cursor = 0;
    int _size = 0;
    TValue _accum_value = DEFAULT_VAL;
    int _accum_count = 0;

public:
    void clear(TTime now, TValue value = DEFAULT_VAL) {
        for (int i = 0; i < HISTORY_SIZE; i++) {
            _history[i].valid = false;
            _history[i].min = value;
            _history[i].max = value;
            _history[i].ave = value;
        }
        
        _first_cursor = _last_cursor = now % PERIOD;
        _size = 0;
        _accum_value = 0;
        _accum_count = 0;
    }

    bool write(TTime now, TValue val) {
        int cursor = (now % PERIOD) / RESO;

        if (!_history[cursor].valid) _size += 1;

        if (_last_cursor == cursor) {
            _accum_value += val;
            _accum_count += 1;
            _history[cursor].valid = true;
            _history[cursor].min = SHPC_MIN(_history[cursor].min, val);
            _history[cursor].max = SHPC_MAX(_history[cursor].max, val);
            _history[cursor].ave = SHPC_ROUND_DIV(_accum_value, _accum_count);
        }
        else {
            _history[cursor].valid = true;
            _history[cursor].min = val;
            _history[cursor].max = val;
            _history[cursor].ave = val;
            _accum_value = val;
            _accum_count = 1;
        }

        bool wrap_around = (cursor < _last_cursor);
        _last_cursor = cursor;
        return wrap_around;
    }

    int size() const { return _size; }
    int empty() const { return _size == 0; }
    int full() const { return _size == HISTORY_SIZE; }

    void get_min_max(TValue *min_val, TValue *max_val, TValue default_value = DEFAULT_VAL, bool use_ave = false) const {
        TValue min_hold = default_value;
        TValue max_hold = default_value;
        for (int i = 0; i < HISTORY_SIZE; i++) {
            if (!_history[i].valid) continue;
            TValue this_min = use_ave ? _history[i].ave : _history[i].min;
            TValue this_max = use_ave ? _history[i].ave : _history[i].max;
            if (i == 0) {
                min_hold = this_min;
                max_hold = this_max;
            }
            else {
                min_hold = SHPC_MIN(min_hold, this_min);
                max_hold = SHPC_MAX(max_hold, this_max);
            }
        }
        if (min_val) *min_val = min_hold;
        if (max_val) *max_val = max_hold;
    }

    TValue min(TValue default_value = DEFAULT_VAL, bool use_ave = false) const {
        TValue min_val;
        get_min_max(&min_val, nullptr, default_value, use_ave);
        return min_val;
    }

    TValue max(TValue default_value = DEFAULT_VAL, bool use_ave = false) const {
        TValue max_val;
        get_min_max(nullptr, &max_val, default_value, use_ave);
        return max_val;
    }

    TValue total_amplitude(TValue default_value = DEFAULT_VAL, bool use_ave = false) const {
        TValue min_val, max_val;
        get_min_max(&min_val, &max_val, default_value, use_ave);
        return max_val - min_val;
    }

    TValue total_average(TValue default_value = DEFAULT_VAL) const { 
        if (_size == 0) return default_value;
        TValue accum = 0;
        for (int i = 0; i < HISTORY_SIZE; i++) {
            if (!_history[i].valid) continue;
            accum += _history[i].ave;
        }
        return SHPC_ROUND_DIV(accum, _size);
    }

    TValue average_amplitude(TValue default_value = DEFAULT_VAL, bool use_ave = false) const {
        if (_size == 0) return default_value;
        TValue accum = 0;
        for (int i = 0; i < HISTORY_SIZE; i++) {
            if (!_history[i].valid) continue;
            accum += _history[i].max - _history[i].min;
        }
        return SHPC_ROUND_DIV(accum, _size);
    }
};

}
