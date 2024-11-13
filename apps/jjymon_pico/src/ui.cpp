#include <stdint.h>
#include <stdio.h>

#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include "jjymon.hpp"
#include "ui.hpp"

#include "shapoco/graphics/graphics.hpp"
#include "shapoco/pico/atomic.hpp"
#include "shapoco/ssd1306/pico/spi_lcd.hpp"
#include "shapoco/jjy/jjy.hpp"
#include "shapoco/fixed12.hpp"

#include "images.hpp"
#include "fonts.hpp"

#include "rader.hpp"
#include "buffer_view.hpp"

#include "shapoco/lazy_timer.hpp"

namespace shapoco::jjymon {

using namespace ::shapoco::graphics;
using namespace ::shapoco::pico;

extern atomic<receiver_status_t> glb_receiver_status;
static receiver_status_t sts;

static constexpr int FPS = 50;

static JjySpiLcd spiLcd;
static JjyI2cLcd i2cLcd;
static ssd1306::Screen screen(LCD_W, LCD_H);

static Rader rader;
static BufferView bit_table;

static int32_t gain_meter_scale = fxp12::ONE;
static int32_t gain_meter_curr = 0;
static int32_t qty_meter_curr = 0;

static jjy::JjyDateTime goal_date_time;
static jjy::JjyDateTime disp_date_time;

static void render_gain_meter(uint32_t t_now_ms, ssd1306::Screen &screen, int x0, int y0);
static void render_quarity_meter(uint32_t t_now_ms, ssd1306::Screen &screen, int x0, int y0);
static void render_scope(uint64_t t_ms, ssd1306::Screen &screen, int x0, int y0, const receiver_status_t &sts);
static void render_meter(uint32_t t_now_ms, ssd1306::Screen &screen, int x0, int y0, int32_t val);
static void render_date_time(uint64_t t_ms, ssd1306::Screen &screen, const receiver_status_t &sts);

void ui_init(void) {
    uint64_t t = to_us_since_boot(get_absolute_time()) / 1000;
    spiLcd.init();
    i2cLcd.resetI2cBus();
    i2cLcd.init();
    bit_table.init(t);
}

void ui_loop(void) {
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

            screen.clear();

            render_gain_meter(t_now_ms, screen, 0, 0);
            render_quarity_meter(t_now_ms, screen, 0, 18);
            render_scope(t_now_ms, screen, 0, 18 * 2, sts);

            bit_table.render(t_now_ms, screen, LCD_W - bit_table.WIDTH, 0, sts);

            rader.render(t_now_ms, 40, 0, screen, sts);

            render_date_time(t_now_ms, screen, sts);

            spiLcd.commit(screen);
            i2cLcd.commit(screen);
        }

        spiLcd.service();
        i2cLcd.service();
    }
}

static void render_gain_meter(uint32_t t_now_ms, ssd1306::Screen &screen, int x0, int y0) {
    constexpr bool SMOOTH_SCALE = true;

    if (SMOOTH_SCALE) {
        gain_meter_scale = fxp12::follow(gain_meter_scale, sts.rf.agc_gain, fxp12::ONE / 4);
    }
    else {
        if (sts.rf.agc_gain > gain_meter_scale * 3 / 2) {
            gain_meter_scale *= 2;
        }
        else if (sts.rf.agc_gain < gain_meter_scale * 3 / 4) {
            gain_meter_scale /= 2;
        }
    }

    //int32_t goal = sts.rf.adc_amplitude_raw * gain_meter_scale / (jjy::rx::Agc_Deprecated::GOAL_AMPLITUDE * 5 / 4);
    int32_t goal = sts.rf.adc_amplitude_raw * gain_meter_scale / (jjy::ONE * 5 / 4);
    gain_meter_curr = fxp12::follow(gain_meter_curr, goal, fxp12::ONE / 2);
    
    screen.draw_string(fonts::font5, x0, y0, "AMP");

    const int scale_text_x = x0 + 17;
    char s[8];
    if (SMOOTH_SCALE) {
        sprintf(s, "x%3.1f", (float)gain_meter_scale / jjy::ONE);
    }
    else {
        if (gain_meter_scale >= jjy::ONE) {
            sprintf(s, "x%1d", (int)(gain_meter_scale / jjy::ONE));
        }
        else {
            sprintf(s, "/%1d", (int)(jjy::ONE / gain_meter_scale));
        }
    }
    screen.draw_string(fonts::font5, scale_text_x, y0, s);

    render_meter(t_now_ms, screen, 0, y0 + 6, gain_meter_curr);
}

static void render_quarity_meter(uint32_t t_now_ms, ssd1306::Screen &screen, int x0, int y0) {
    qty_meter_curr = fxp12::follow(qty_meter_curr, sts.sync.bit_det_quality, fxp12::ONE / 8);

    //int32_t qty = qty_meter_curr * sts.rf.signal_quarity / jjy::ONE;
    int32_t qty = (qty_meter_curr + sts.rf.signal_quarity) / 2;
    //int32_t qty = sts.rf.signal_quarity;

    screen.draw_string(fonts::font5, x0, y0, "QTY");
    render_meter(t_now_ms, screen, 0, y0 + 6, qty);
    
    char s[8];
    sprintf(s, "%d", (int)(qty * 100 / jjy::ONE));
    screen.draw_string(fonts::font5, x0 + 22, y0, s);
}

static void render_scope(uint64_t t_ms, ssd1306::Screen &screen, int x0, int y0, const receiver_status_t &sts) {
    int w = jjy::rx::DifferentialDetector::SCOPE_SIZE;
    const uint32_t *scope = sts.scope;
    for (int x = 0; x < w; x++) {
        uint32_t val = scope[x];
        for (int y = 0; y < 8; y++) {
            val = (val >> 2) | (val & 3);
            val = (val >> 1) | (val & 1);
            if (val & 1) {
                screen.set_pixel(x0 + x, y0 + y);
            }
            val >>= 1;
        }

    }
}

static void render_meter(uint32_t t_now_ms, ssd1306::Screen &screen, int x0, int y0, int32_t val) {
    constexpr int32_t A_PERIOD = fxp12::PHASE_PERIOD * 45 / 360;
    constexpr int RADIUS_MAX = 40;
    constexpr int RADIUS_MIN = RADIUS_MAX - 10;

    screen.draw_bitmap(x0, y0, bmp_meter_frame);
    
    val = FXP_CLIP(0, fxp12::ONE, val);
    int32_t a = (fxp12::PHASE_PERIOD * 3 / 4 - A_PERIOD / 2) + (A_PERIOD * val) / fxp12::ONE;
    int32_t sin = fxp12::fast_sin(a);
    int32_t cos = fxp12::fast_cos(a);
    int32_t cx = (x0 + 16) * fxp12::ONE;
    int32_t cy = (y0 + RADIUS_MAX) * fxp12::ONE + fxp12::ONE / 2;
    int32_t lx0 = cx + cos * (RADIUS_MAX - 1);
    int32_t ly0 = cy + sin * (RADIUS_MAX - 1);
    int32_t lx1 = cx + cos * RADIUS_MIN;
    int32_t ly1 = cy + sin * RADIUS_MIN;
    screen.draw_line_f(lx0 - fxp12::ONE / 2, ly0, lx1 - fxp12::ONE / 2, ly1);
    screen.draw_line_f(lx0 + fxp12::ONE / 2, ly0, lx1 + fxp12::ONE / 2, ly1);
}

static void render_date_time(uint64_t t_ms, ssd1306::Screen &lcd, const receiver_status_t &sts) { 
    char s[32];
    const int sts_y = LCD_H - 12 - 6;
    const int guage_x = LCD_W / 2;
    const int guage_w = LCD_W - guage_x;
    bool empty = true;
    if (!sts.sync.phase_locked) {
        lcd.draw_string(fonts::font5, 0, sts_y, "PHASE SYNC...");
        const int guage_val = (LCD_W / 2) * sts.sync.phase_lock_progress / jjy::ONE;
        lcd.draw_rect(guage_x, sts_y, guage_w - 1, 4);
        lcd.fill_rect(guage_x, sts_y + 1, guage_val, 3);
    }
    else if (!sts.dec.synced) {
        lcd.draw_string(fonts::font5, 0, sts_y, "FRAME SYNC...");
        int pos = t_ms % 1024;
        if (pos > 512) pos = 1024 - pos;
        const int guage_pos = (LCD_W * 3 / 8) * pos / 512;
        lcd.draw_rect(guage_x, sts_y, guage_w - 1, 4);
        lcd.fill_rect(guage_x + guage_pos, sts_y + 1, (LCD_W / 8), 3);
    }
    else if (sts.dec.last_parse_result.flags == jjy::ParseResut::EMPTY) {
        lcd.draw_string(fonts::font5, 0, sts_y, "RECEIVING...");
        const int guage_val = (LCD_W / 2) * sts.dec.last_bit_index / 60;
        lcd.draw_rect(guage_x, sts_y, guage_w - 1, 4);
        lcd.fill_rect(guage_x, sts_y + 1, guage_val, 3);
    }
    else if (sts.dec.last_parse_result.success()) {
        //lcd.draw_string(bmpfont::font5, 0, sts_y, "NO ERROR");
        goal_date_time = sts.dec.last_date_time;
        goal_date_time.second = sts.dec.last_bit_index;
        empty = false;
    }
    else {
        sprintf(s, "ERROR CODE = 0x%x", (int)(sts.dec.last_parse_result.flags));
        lcd.draw_string(fonts::font5, 0, sts_y, s);
    }
 
    static int tmp_t = 0;
    if ((tmp_t++) % 3 == 0) {
        disp_date_time.year = (disp_date_time.year / 100 < goal_date_time.year / 100) ? disp_date_time.year + 100 : goal_date_time.year;
        disp_date_time.year = (disp_date_time.year < goal_date_time.year) ? disp_date_time.year + 1 : goal_date_time.year;
        disp_date_time.month = (disp_date_time.month < goal_date_time.month) ? disp_date_time.month + 1 : goal_date_time.month;
        disp_date_time.day = (disp_date_time.day < goal_date_time.day) ? disp_date_time.day + 1 : goal_date_time.day;
        disp_date_time.hours = (disp_date_time.hours < goal_date_time.hours) ? disp_date_time.hours + 1 : goal_date_time.hours;
        disp_date_time.minutes = (disp_date_time.minutes < goal_date_time.minutes) ? disp_date_time.minutes + 1 : goal_date_time.minutes;
        disp_date_time.second = (disp_date_time.second < goal_date_time.second) ? disp_date_time.second + 1 : goal_date_time.second;
    }

    if (empty) {
        lcd.draw_string(fonts::font12, 0, LCD_H - 12, "----/--/-- --:--:--");
    }
    else {
        sprintf(s, "%04d/%02d/%02d %02d:%02d:%02d",
            disp_date_time.year,
            disp_date_time.month - jjy::MONTH_OFFSET + 1,
            disp_date_time.day - jjy::DAY_OFFSET + 1,
            disp_date_time.hours,
            disp_date_time.minutes,
            sts.dec.last_bit_index);
        lcd.draw_string(fonts::font12, 0, LCD_H - 12, s);
    }
}

}
