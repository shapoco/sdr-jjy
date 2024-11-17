#include <math.h>

#include "shapoco/fixed12.hpp"

namespace shapoco::fxp12 {

static bool table_inited = false;
int32_t sin_table[PHASE_PERIOD];

void initTables(void) {
    if (table_inited) return;
    for (int a = 0; a < PHASE_PERIOD; a++) {
        sin_table[a] = ::round(::sin(a * 2 * M_PI / PHASE_PERIOD) * ONE);
    }
    table_inited = true;
}

int32_t fastSin(int32_t a) {
    initTables();
    return sin_table[phaseNorm(a)];
}

int32_t fastCos(int32_t a) {
    initTables();
    return sin_table[phaseNorm(a + PHASE_PERIOD / 4)];
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
