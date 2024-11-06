#include <stdint.h>
#include <stdio.h>

#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include "jjymon.hpp"

#include "core0.hpp"
#include "core1.hpp"

#include "atomic.hpp"
#include "jjy/jjy.hpp"
#include "fixed12.hpp"

#include "ssd1309spi.hpp"
#include "images/images.hpp"
#include "bmpfont/bmpfont.hpp"

#include "meter.hpp"
#include "rader.hpp"
#include "bit_log_table.hpp"

#include "lazy_timer.hpp"

#include "lcd_demo.hpp"

extern atomic<receiver_status_t> glb_receiver_status;
static receiver_status_t sts;

static constexpr int FPS = 50;

static JjyLcd lcd;
using pen_t = ssd1309spi::pen_t;

static Rader rader;
static Meter amp_meter;
static Meter quarity_meter;
static BitLogTable bit_table;

static int32_t gain_meter_scale = fxp12::ONE;
static int32_t gain_meter_curr = 0;
static int32_t qty_meter_curr = 0;

static void render_gain_meter(uint32_t t_now_ms, int x0, int y0);
static void render_quarity_meter(uint32_t t_now_ms, int x0, int y0);
static void render_meter(uint32_t t_now_ms, int x0, int y0, int32_t val);
static void render_sync_status(uint32_t t_now_ms);

void core1_init() {
    uint64_t t = to_us_since_boot(get_absolute_time()) / 1000;
    lcd.init();
    bit_table.init(t);
}

void core1_main() {
    LazyTimer<uint32_t, 5> waveform_update_timer;
    LazyTimer<uint32_t, 20> render_timer;

    {
        uint32_t t_now_ms = to_ms_since_boot(get_absolute_time());
        render_timer.start(t_now_ms);
    }

    sts = glb_receiver_status.load();

    while (true) {
        uint32_t t_now_ms = to_ms_since_boot(get_absolute_time());

        if (waveform_update_timer.is_expired(t_now_ms)) {
            sts = glb_receiver_status.load();
            rader.update(t_now_ms, sts);
        }

        if (render_timer.is_expired(t_now_ms)) {

            lcd.clear();

#if 0
            lcd_demo_render(lcd, t_now_ms);
#else
            render_gain_meter(t_now_ms, 0, 0);
            render_quarity_meter(t_now_ms, 0, 18);
            rader.render(t_now_ms, 40, 0, lcd, sts);
            render_sync_status(t_now_ms);
            bit_table.render(t_now_ms, lcd, LCD_W - 5 * 10 + 1, 0, sts);
#endif
            lcd.commit();
        }

        lcd.service();
    }
}

static void render_gain_meter(uint32_t t_now_ms, int x0, int y0) {
    constexpr bool SMOOTH_SCALE = true;

    if (SMOOTH_SCALE) {
        fxp12::interp(&gain_meter_scale, sts.rf.agc_gain, fxp12::ONE / 4);
    }
    else {
        if (sts.rf.agc_gain > gain_meter_scale * 3 / 2) {
            gain_meter_scale *= 2;
        }
        else if (sts.rf.agc_gain < gain_meter_scale * 3 / 4) {
            gain_meter_scale /= 2;
        }
    }

    int32_t goal = sts.rf.adc_amplitude_raw * gain_meter_scale / (jjy::rx::Agc::GOAL_AMPLITUDE * 5 / 4);
    fxp12::interp(&gain_meter_curr, goal, fxp12::ONE / 2);
    
    lcd.draw_string(bmpfont::font5, x0, y0, "AMP");

    const int scale_text_x = x0 + 17;
    char s[8];
    if (SMOOTH_SCALE) {
        sprintf(s, "x%3.1f", (float)gain_meter_scale / jjy::ONE);
    }
    else {
        if (gain_meter_scale >= jjy::ONE) {
            sprintf(s, "x%1d", gain_meter_scale / jjy::ONE);
        }
        else {
            sprintf(s, "/%1d", jjy::ONE / gain_meter_scale);
        }
    }
    lcd.draw_string(bmpfont::font5, scale_text_x, y0, s);

    render_meter(t_now_ms, 0, y0 + 6, gain_meter_curr);
}

static void render_quarity_meter(uint32_t t_now_ms, int x0, int y0) {
    fxp12::interp(&qty_meter_curr, sts.sync.bit_det_quality, fxp12::ONE / 8);
#if 1
    int32_t qty = qty_meter_curr * sts.rf.signal_quarity / jjy::ONE;
#else
    int32_t qty = (qty_meter_curr + sts.rf.signal_quarity) / 2;
#endif

    lcd.draw_string(bmpfont::font5, x0, y0, "QTY");
    render_meter(t_now_ms, 0, y0 + 6, qty);
    
    char s[8];
    sprintf(s, "%d", qty * 100 / jjy::ONE);
    lcd.draw_string(bmpfont::font5, x0 + 22, y0, s);
}

static void render_meter(uint32_t t_now_ms, int x0, int y0, int32_t val) {
    constexpr int32_t A_PERIOD = fxp12::ANGLE_PERIOD * 45 / 360;
    constexpr int RADIUS_MAX = 40;
    constexpr int RADIUS_MIN = RADIUS_MAX - 10;

    lcd.draw_bitmap(x0, y0, bmp_meter_frame);
    
    val = FXP_CLIP(0, fxp12::ONE, val);
    int32_t a = (fxp12::ANGLE_PERIOD * 3 / 4 - A_PERIOD / 2) + (A_PERIOD * val) / fxp12::ONE;
    int32_t sin = fxp12::sin(a);
    int32_t cos = fxp12::cos(a);
    int32_t cx = (x0 + 16) * fxp12::ONE;
    int32_t cy = (y0 + RADIUS_MAX) * fxp12::ONE + fxp12::ONE / 2;
    int32_t lx0 = cx + cos * (RADIUS_MAX - 1);
    int32_t ly0 = cy + sin * (RADIUS_MAX - 1);
    int32_t lx1 = cx + cos * RADIUS_MIN;
    int32_t ly1 = cy + sin * RADIUS_MIN;
    lcd.draw_line_f(lx0 - fxp12::ONE / 2, ly0, lx1 - fxp12::ONE / 2, ly1);
    lcd.draw_line_f(lx0 + fxp12::ONE / 2, ly0, lx1 + fxp12::ONE / 2, ly1);
}

static void render_sync_status(uint32_t t_now_ms) {
    int x = 0;
    int y = 40;
    const bmpfont::Font &font = bmpfont::font16;
    if (!sts.sync.phase_locked) {
        lcd.draw_string(font, x, y, "Phase Sync...");
        y += font.height - 2;
        lcd.fill_rect(x, y, (LCD_W / 2) * sts.sync.phase_lock_progress / jjy::ONE, 2);
    }
    else if (!sts.sync.bit_det_ok) {
        lcd.draw_string(font, x, y, "Pulse Check...");
        y += font.height - 2;
        lcd.fill_rect(x, y, (LCD_W / 2) + (LCD_W / 2) * sts.sync.bit_det_progress / jjy::ONE, 2);
    }
}