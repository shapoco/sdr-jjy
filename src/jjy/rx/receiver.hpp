#ifndef JJY_RX_RECEIVER_HPP
#define JJY_RX_RECEIVER_HPP

#include "jjy/common.hpp"
#include "jjy/rx/radio.hpp"
#include "jjy/rx/synchronizer.hpp"
#include "jjy/rx/decoder.hpp"

namespace jjy::rx {

class Receiver {
public:
    Rf rf;
    Synchronizer sync;
    Decoder dec;

    Receiver(uint32_t dma_size) : rf(dma_size) { }

    void init(freq_t freq, uint32_t t_now_ms) {
        rf.init(freq, t_now_ms);
        sync.init(t_now_ms);
        dec.init(t_now_ms);
    }

    void process(const uint32_t t_now_ms, const uint16_t *samples) {
        // 検波
        uint8_t signal = rf.process(t_now_ms, samples);

        // ビット同期
        jjybit_t bit;
        bool tick = sync.process(t_now_ms, signal, &bit);

        if (tick) {
            // デコード
            dec.process(t_now_ms, bit);
        }
    }
};

}

#endif
