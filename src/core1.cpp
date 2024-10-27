#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include "core0.hpp"
#include "core1.hpp"

static uint8_t vram[BARLED_NUM_SEGS];

static constexpr int SCAN_INTERVAL_US = 200;

static void animate_bit_scan(uint64_t t_now);

void core1_init() {
    for (int seg = 0; seg < BARLED_NUM_SEGS; seg++) {
        vram[seg] = 0;
    }
    for (int seg = 0; seg < BARLED_NUM_SEGS; seg++) {
        gpio_init(PIN_BARLED_SEG_BASE + seg);
        gpio_put(PIN_BARLED_SEG_BASE + seg, false);
        gpio_set_dir(PIN_BARLED_SEG_BASE + seg, GPIO_IN);
    }
    for (int ch = 0; ch < BARLED_NUM_CHS; ch++) {
        gpio_init(PIN_BARLED_CH_BASE + ch);
        gpio_put(PIN_BARLED_CH_BASE + ch, false);
        gpio_set_dir(PIN_BARLED_CH_BASE + ch, GPIO_OUT);
    }
}

void core1_main() {
    int seg = 0;
    uint64_t t_next_upd = to_us_since_boot(get_absolute_time()) + SCAN_INTERVAL_US;
    while (true) {
        uint8_t c = vram[BARLED_NUM_SEGS - seg - 1];
        gpio_put(PIN_BARLED_CH_BASE + BARLED_CH_R, !(c & 1));
        gpio_put(PIN_BARLED_CH_BASE + BARLED_CH_G, !(c & 2));
        gpio_put(PIN_BARLED_CH_BASE + BARLED_CH_B, !(c & 4));
        gpio_put(PIN_BARLED_SEG_BASE + seg, true);
        gpio_set_dir(PIN_BARLED_SEG_BASE + seg, GPIO_OUT);
        
        uint64_t t_now;
        while ((t_now = to_us_since_boot(get_absolute_time())) < t_next_upd) {
            animate_bit_scan(t_now);
            sleep_us(10);
        }

        t_next_upd += SCAN_INTERVAL_US;
        gpio_set_dir(PIN_BARLED_SEG_BASE + seg, GPIO_IN);
        seg = (seg < BARLED_NUM_SEGS - 1) ? seg + 1 : 0;
    }
}

static void animate_bit_scan(uint64_t t_now) {
    static int last_cursor = -1;
    static uint8_t last_signal = 0;
    static bool all0 = true;
    static bool all1 = false;
    static bool rise = false;
    static bool fall = false;

    int curr_cursor = (t_now / 100000) % BARLED_NUM_SEGS;
    uint8_t curr_signal = glb_jjy_curr_signal;

    if (curr_cursor != last_cursor) {
        if (last_cursor >= 0) {
            uint8_t c = 1;
            if (all0) c = 0;
            else if (all1) c = 7;
            else if (rise && !fall) c = 7;
            else if (!rise && fall) c = 0;
            else if (rise && fall) c = 1;
            vram[last_cursor] = c;
        }
        all0 = true;
        all1 = true;
        rise = false;
        fall = false;
        vram[curr_cursor] = 4;
    }

    if (curr_signal & ~last_signal) rise = true;
    else if (~curr_signal & last_signal) fall = true;

    if (curr_signal) all0 = false;
    else all1 = false;

    last_cursor = curr_cursor;
    last_signal = curr_signal;
}