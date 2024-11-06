#ifndef JJY_RX_DECODER_HPP
#define JJY_RX_DECODER_HPP

#include <stdint.h>
#include <math.h>
#include <string.h>

#include "pico/stdlib.h"

#include "shapoco/ring_scope.hpp"
#include "shapoco/peak_hold.hpp"

#include "jjy/common.hpp"

using namespace shapoco; // todo: 削除

namespace jjy::rx {

class Decoder {
public:
    static constexpr int NUM_PRE_SYNC = 3;

    enum class action_t {
        UNLOCK,
        SYNC_MARKER,
        SYNC_CONTINUE,
        SYNC_FINISH,
        TICK_CONTINUE,
        TICK_WRAP,
    };

    struct status_t {
        bool toggle = false;
        action_t last_action;
        int num_marker_found;    
        bool synced;
        int last_bit_index;
        jjybit_t last_bit_value;

        void init(uint32_t t_now_ms) {
            last_action = action_t::UNLOCK;
            num_marker_found = 0;
            synced = false;
            last_bit_index = 0;
            last_bit_value = jjybit_t::ERROR;
        }
    };

private:
    status_t sts;
    int bit_index;

public:

    void init(uint32_t t_now_ms) {
        sts.init(t_now_ms);
        bit_index = 0;
    }

    action_t process(uint32_t t_now_ms, jjybit_t in) {
        action_t new_action = action_t::UNLOCK;
        
        sts.last_bit_index = bit_index;

        if (in == jjybit_t::ERROR) {
            new_action = action_t::UNLOCK;
        }
        else if (!sts.synced) {
            if (sts.num_marker_found == 0 && in == jjybit_t::MARKER) {
                new_action = action_t::SYNC_MARKER;
            }
            else if (bit_index == 0 && sts.num_marker_found >= NUM_PRE_SYNC && in == jjybit_t::MARKER) {
                new_action = action_t::SYNC_FINISH;
            }
            else if (bit_index == 0) {
                new_action = action_t::SYNC_CONTINUE;
            }
            else if (bit_index == 8 && sts.num_marker_found == 1 && in == jjybit_t::MARKER) {
                new_action = action_t::SYNC_MARKER;
            }
            else if (bit_index < 9 && in != jjybit_t::MARKER) {
                new_action = action_t::SYNC_CONTINUE;
            }
            else if (bit_index == 9 && in == jjybit_t::MARKER) {
                new_action = action_t::SYNC_MARKER;
            }
            else {
                new_action = action_t::UNLOCK;
            }
        }
        else {
            int bi10 = bit_index / 10;
            int bi1 = bit_index % 10;
            if (bit_index == 0 && in == jjybit_t::MARKER) {
                new_action = action_t::TICK_CONTINUE;
            }
            else if (bit_index == 59 && in == jjybit_t::MARKER) {
                new_action = action_t::TICK_WRAP;
            }
            else if (bi1 < 9 && in != jjybit_t::MARKER) {
                new_action = action_t::TICK_CONTINUE;
            }
            else if (bi1 == 9 && in == jjybit_t::MARKER) {
                new_action = action_t::TICK_CONTINUE;
            }
            else {
                new_action = action_t::UNLOCK;
            }
        }

        bool accept = false;

        switch(new_action) {
        case action_t::SYNC_MARKER:
            bit_index = 0;
            sts.synced = false;
            sts.num_marker_found += 1;
            break;

        case action_t::SYNC_CONTINUE:
            bit_index += 1;
            sts.synced = false;
            break;
        
        case action_t::SYNC_FINISH:
            bit_index = 1;
            sts.synced = true;
            accept = true;
            break;

        case action_t::TICK_CONTINUE:
            bit_index += 1;
            sts.synced = true;
            accept = true;
            break;

        case action_t::TICK_WRAP:
            bit_index = 0;
            sts.synced = true;
            accept = true;
            break;

        case action_t::UNLOCK:
        default:
            bit_index = 0;
            sts.synced = false;
            sts.num_marker_found = 0;
            break;
        }

        sts.last_bit_value = in;
        sts.last_action = new_action;
        sts.toggle = !sts.toggle;

        return new_action;
    }

    const status_t &get_status() const {
        return (const status_t &)sts;
    }
};

}

#endif
