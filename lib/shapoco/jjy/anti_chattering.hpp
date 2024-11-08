#ifndef SHAPOCO_JJY_ANTI_CHATTERING_HPP
#define SHAPOCO_JJY_ANTI_CHATTERING_HPP

#include <stdint.h>

namespace jjy {

template<int DEPTH>
class AntiChattering {
public:
    uint32_t sreg;
    uint8_t out;
    
public:
    void reset() {
        sreg = 0;
        out = 0;
    }
    
    uint8_t process(uint8_t in) {
        sreg = (sreg << 1) & ((1 << DEPTH) - 1);
        sreg |= in;
        if (sreg == 0) {
            out = 0;
        }
        else if (sreg == ((1 << DEPTH) - 1)) {
            out = 1;
        }
        return out;
    }
};

}

#endif
