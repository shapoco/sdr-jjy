#ifndef JJY_RX_RECEIVER_HPP
#define JJY_RX_RECEIVER_HPP

#include "jjy/common.hpp"
#include "jjy/rx/radio.hpp"
#include "jjy/rx/bit_sync.hpp"
#include "jjy/rx/decoder.hpp"

namespace jjy::rx {

class Receiver {
public:
    Rf rf;

    void init(freq_t freq) {
        rf.init(freq);
    }

    void receive(const uint32_t t_now_ms, const uint16_t *samples, const uint32_t size) {
        rf.detect(t_now_ms, samples, size);
    }
};

}

#endif
