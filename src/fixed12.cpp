#include <math.h>

#include "fixed12.hpp"

namespace fxp12 {

static bool table_inited = false;
int32_t sin_table[ANGLE_PERIOD];
int32_t cos_table[ANGLE_PERIOD];

void init_tables(void) {
    if (table_inited) return;
    for (int a = 0; a < ANGLE_PERIOD; a++) {
        sin_table[a] = round(sin(a * 2 * M_PI / ANGLE_PERIOD) * ONE);
        cos_table[a] = round(cos(a * 2 * M_PI / ANGLE_PERIOD) * ONE);
    }
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

}