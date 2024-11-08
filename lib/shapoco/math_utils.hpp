#ifndef SHAPOCO_MATH_UTILS_HPP
#define SHAPOCO_MATH_UTILS_HPP

#include <stdint.h>

#include "shapoco/common.hpp"

#define SHPC_ABS(x) ((x) >= 0 ? (x) : -(x))
#define SHPC_MIN(a, b) ((a) < (b) ? (a) : (b))
#define SHPC_MAX(a, b) ((a) > (b) ? (a) : (b))
#define SHPC_CLIP(min, max, val) (SHPC_MAX((min), SHPC_MIN((max), (val))))

#define SHPC_CEIL_DIV(a, b) (((a) + (b) - 1) / (b))
#define SHPC_ROUND_DIV(a, b) (((a) + ((b) / 2)) / (b))

#endif
