#pragma once

#include <stdint.h>

#include "shapoco/common.hpp"

#define SHPC_ABS(x) ((x) >= 0 ? (x) : -(x))
#define SHPC_MIN(a, b) ((a) < (b) ? (a) : (b))
#define SHPC_MAX(a, b) ((a) > (b) ? (a) : (b))
#define SHPC_CLIP(min, max, val) (SHPC_MAX((min), SHPC_MIN((max), (val))))

#define SHPC_CEIL_DIV(a, b) (((a) + (b) - 1) / (b))
#define SHPC_ROUND_DIV(a, b) (((a) + ((b) / 2)) / (b))

#define SHPC_CYCLIC_INCR(x, period) (((x) + 1 < (period)) ? ((x) + 1) : 0)
#define SHPC_CYCLIC_DECR(x, period) (((x) - 1 >= 0) ? ((x) - 1) : (period) - 1)

namespace shapoco {

static SHPC_INLINE int clog10(uint32_t x) {
    if (x <= (uint32_t)1e5)
        if (x <= (uint32_t)1e3)
            if (x <= (uint32_t)1e1)
                if (x <= (uint32_t)1e0)
                    return 0;
                else
                    return 1;
            else 
                if (x <= (uint32_t)1e2)
                    return 2;
                else
                    return 3;
        else
            if (x <= (uint32_t)1e4)
                return 4;
            else
                return 5;
    else 
        if (x <= (uint32_t)1e7)
            if (x <= (uint32_t)1e6)
                return 6;
            else
                return 7;
        else
            if (x <= (uint32_t)1e8)
                return 8;
            else
                if (x <= (uint32_t)1e9)
                    return 9;
                else
                    return 10;
}

}

