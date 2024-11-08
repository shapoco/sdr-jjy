#include <math.h>

#include "shapoco/fixed12.hpp"

namespace shapoco::fxp12 {

static bool table_inited = false;
static int32_t sin_table[ANGLE_PERIOD];
static int32_t cos_table[ANGLE_PERIOD];

void init_tables(void) {
    if (table_inited) return;
    for (int a = 0; a < ANGLE_PERIOD; a++) {
        sin_table[a] = round(::sin(a * 2 * M_PI / ANGLE_PERIOD) * ONE);
        cos_table[a] = round(::cos(a * 2 * M_PI / ANGLE_PERIOD) * ONE);
    }
    table_inited = true;
}

int32_t normalize_angle(int32_t a) {
    if (a < 0) {
        do { a += ANGLE_PERIOD; } while(a < 0);
    }
    else if (a >= ANGLE_PERIOD) {
        do { a -= ANGLE_PERIOD; } while(a >= ANGLE_PERIOD);
    }
    return a;
}

int32_t sin(int32_t a) {
    init_tables();
    return sin_table[normalize_angle(a)];
}

int32_t cos(int32_t a) {
    init_tables();
    return cos_table[normalize_angle(a)];
}

int32_t log2(int32_t x) {
    if (x <= 1) return 0x80000000;

    uint32_t tmp = x;
    int32_t ret = 0;
    if (tmp > ONE * 2) {
        do { tmp /= 2; ret += 1; } while (tmp > ONE * 2);
    }
    else if (tmp < ONE) {
        do { tmp *= 2; ret -= 1; } while (tmp < ONE);
    }

    for (int i = 0; i < PREC; i++) {
        tmp = tmp * tmp / ONE;
        ret *= 2;
        if (tmp >= 2 * ONE) {
            ret += 1;
            tmp /= 2;
        }
    }

    return ret;
}

}
