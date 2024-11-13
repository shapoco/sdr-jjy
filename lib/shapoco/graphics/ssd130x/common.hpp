#pragma once

#include <stdint.h>

namespace shapoco::graphics::ssd130x {

using seg_t = uint8_t;

typedef enum : uint8_t {
    BLACK = 0,
    WHITE = 1,
} pen_t;

}
