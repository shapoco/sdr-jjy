#pragma once

#include <stdint.h>

#include "shapoco/common.hpp"

namespace shapoco {

SHPC_STATIC_INLINE int clog10(uint32_t x) {
    if (x <= (uint32_t)1e5) {
        if (x <= (uint32_t)1e3) {
            if (x <= (uint32_t)1e1) {
                return (x <= (uint32_t)1e0) ? 0 : 1;
            }
            else {
                return (x <= (uint32_t)1e2) ? 2 : 3;
            }
        }
        else {
            return (x <= (uint32_t)1e4) ? 4 : 5;
        }
    }
    else {
        if (x <= (uint32_t)1e7) {
            return (x <= (uint32_t)1e6) ? 6 : 7;
        }
        else {
            if (x <= (uint32_t)1e8) {
                return 8;
            }
            else {
                return (x <= (uint32_t)1e9) ? 9 : 10;
            }
        }
    }
}

}

