#pragma once

#include <stdint.h>
#include <math.h>
#include <string.h>

#include "pico/stdlib.h"

#include "shapoco/ring_scope.hpp"
#include "shapoco/peak_hold.hpp"

#include "shapoco/jjy/common.hpp"
#include "shapoco/jjy/datetime.hpp"
#include "shapoco/jjy/rx/common.hpp"

namespace shapoco::jjy::rx {

class Decoder {
public:
    static constexpr int INITIAL_BIT_INDEX = 59;

    enum class action_t {
        ABORT,
        SYNC_POS_MARKER,
        SYNC_MARKER,
        TICK_CONTINUE,
        TICK_WRAP,
    };

    struct status_t {
        bool toggle = false;
        bool synced;
        action_t last_action;
        int last_bit_index;
        jjybit_t last_bit_value;
        JjyDateTime last_date_time;
        ParseResut last_parse_result;

        void init(uint32_t t_now_ms) {
            last_action = action_t::ABORT;
            synced = false;
            last_bit_index = 0;
            last_bit_value = jjybit_t::ERROR;
            last_parse_result.flags = ParseResut::EMPTY;
        }
    };

private:
    status_t sts;
    jjybit_t rxBuff[60];
    int bit_index = INITIAL_BIT_INDEX;

public:

    void init(uint32_t t_now_ms) {
        sts.init(t_now_ms);
        memset(rxBuff, (int)jjybit_t::ERROR, sizeof(jjybit_t) * 60);
        bit_index = INITIAL_BIT_INDEX;
    }

    action_t process(uint32_t t_now_ms, jjybit_t in) {
        action_t new_action = action_t::ABORT;
        
        sts.last_bit_index = bit_index;

        if (in == jjybit_t::ERROR) {
            new_action = action_t::ABORT;
        }
        else if (!sts.synced) {
            if (bit_index == INITIAL_BIT_INDEX && in == jjybit_t::MARKER) {
                new_action = action_t::SYNC_POS_MARKER;
            }
            else if (bit_index == 0 && in == jjybit_t::MARKER) {
                new_action = action_t::SYNC_MARKER;
            }
            else {
                new_action = action_t::ABORT;
            }
        }
        else {
            int mod10 = bit_index % 10;
            if (bit_index == 0 && in == jjybit_t::MARKER) {
                new_action = action_t::TICK_CONTINUE;
            }
            else if (bit_index == 59 && in == jjybit_t::MARKER) {
                new_action = action_t::TICK_WRAP;
            }
            else if (mod10 < 9 && in != jjybit_t::MARKER) {
                new_action = action_t::TICK_CONTINUE;
            }
            else if (mod10 == 9 && in == jjybit_t::MARKER) {
                new_action = action_t::TICK_CONTINUE;
            }
            else {
                new_action = action_t::ABORT;
            }
        }

        bool accept = false;

        switch(new_action) {
        case action_t::SYNC_POS_MARKER:
            bit_index = 0;
            sts.synced = false;
            break;

        case action_t::SYNC_MARKER:
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

        case action_t::ABORT:
        default:
            bit_index = INITIAL_BIT_INDEX;
            sts.synced = false;
            break;
        }

        if (accept && 0 <= sts.last_bit_index && sts.last_bit_index < 60) {
            rxBuff[sts.last_bit_index] = in;
            if (new_action == action_t::TICK_WRAP) {
                sts.last_parse_result = sts.last_date_time.parse(rxBuff, 2000);
                sts.last_date_time.addSecondsSelf(60);
            }
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
