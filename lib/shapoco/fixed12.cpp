#include <math.h>

#include "shapoco/fixed12.hpp"

namespace shapoco::fxp12 {

static bool table_inited = false;
int32_t sin_table[PHASE_PERIOD];

void init_tables(void) {
    if (table_inited) return;
    for (int a = 0; a < PHASE_PERIOD; a++) {
        sin_table[a] = round(sin(a * 2 * M_PI / PHASE_PERIOD) * ONE);
    }
    table_inited = true;
}

int32_t fast_sin(int32_t a) {
    init_tables();
    return sin_table[phase_norm(a)];
}

int32_t fast_cos(int32_t a) {
    init_tables();
    return sin_table[phase_norm(a + PHASE_PERIOD / 4)];
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
