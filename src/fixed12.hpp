#ifndef FIXED12_HPP
#define FIXED12_HPP

#include <stdint.h>

namespace fxp12 {

constexpr int PREC = 12;
constexpr int32_t ONE = 1 << PREC;
constexpr int32_t ANGLE_PERIOD = ONE;

void init_tables(void);
int32_t normalize_angle(int32_t a);
int32_t sin(int32_t a);
int32_t cos(int32_t a);

}

#endif
