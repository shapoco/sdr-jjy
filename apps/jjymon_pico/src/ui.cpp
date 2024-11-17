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

#include "rotary_counter.hpp"
//#include "rader.hpp"
#include "pulse_view.hpp"
#include "buffer_view.hpp"
#include "clock_view.hpp"

#include "shapoco/lazy_timer.hpp"

namespace shapoco::jjymon {

using namespace shapoco::graphics;
using namespace shapoco::pico;

extern atomic<receiver_status_t> glb_receiver_status;
static receiver_status_t sts;

static constexpr int FPS = 50;

static JjySpiLcd spiLcd;
static JjyI2cLcd i2cLcd;
static ssd1306::Screen g(LCD_W, LCD_H);

static RotaryCounter ampNumber(fonts::font5, 0, 100);
static RotaryCounter qtyNumber(fonts::font5, 0, 100);

static PulseView pulseView;
static BufferView buffView;
static ClockView clockView;

static int32_t gain_meter_curr = 0;
static int32_t qty_meter_curr = 0;

static void render_gain_meter(uint64_t nowMs, ssd1306::Screen &g, int x0, int y0);
static void render_quarity_meter(uint64_t nowMs, ssd1306::Screen &g, int x0, int y0);
static void render_scope(uint64_t nowMs, ssd1306::Screen &g, int x0, int y0, const receiver_status_t &sts);
static void render_meter(uint64_t nowMs, ssd1306::Screen &g, int x0, int y0, int32_t val);
//static void render_date_time(uint64_t nowMs, ssd1306::Screen &g, const receiver_status_t &sts);

void ui_init(void) {
    uint64_t nowMs = to_us_since_boot(get_absolute_time()) / 1000;
    spiLcd.init();
    i2cLcd.resetI2cBus();
    i2cLcd.init();
    pulseView.init(nowMs);
    buffView.init(nowMs);
}

void ui_loop(void) {
    LazyTimer<uint32_t, 5> waveform_update_timer;
    LazyTimer<uint32_t, 20> render_timer;

    {
        uint64_t nowMs = to_us_since_boot(get_absolute_time()) / 1000;
        render_timer.start(nowMs);
    }

    sts = glb_receiver_status.load();

    while (true) {
        uint64_t nowMs = to_us_since_boot(get_absolute_time()) / 1000;

        if (waveform_update_timer.is_expired(nowMs)) {
            sts = glb_receiver_status.load();
            //rader.update(t_ms, sts);
            pulseView.update(nowMs, sts);
        }

        if (render_timer.is_expired(nowMs)) {

            g.clear();

            render_gain_meter(nowMs, g, 0, 0);
            render_quarity_meter(nowMs, g, 0, 18);
            render_scope(nowMs, g, 0, 18 * 2, sts);

            buffView.render(nowMs, g, LCD_W - buffView.WIDTH, 0, sts);

            //rader.render(t_ms, 40, 0, g, sts);
            pulseView.render(nowMs, g, 38, 0);

            //render_date_time(nowMs, g, sts);
            
            clockView.update(nowMs, sts);
            clockView.render(g, 0, LCD_H - ClockView::HEIGHT);

            spiLcd.commit(g);
            i2cLcd.commit(g);
        }

        spiLcd.service();
        i2cLcd.service();
    }
}

static void render_gain_meter(uint64_t nowMs, ssd1306::Screen &g, int x0, int y0) {
    //int32_t goal = sts.rf.adc_amplitude_raw * gain_meter_scale / (jjy::rx::Agc_Deprecated::GOAL_AMPLITUDE * 5 / 4);
    //int32_t goal = sts.rf.adc_amplitude_raw * gain_meter_scale / (jjy::ONE * 5 / 4);
    int32_t goal = SHPC_ROUND_DIV(fxp12::log2(sts.rf.adc_amplitude_raw * fxp12::ONE), 12);

    gain_meter_curr = fxp12::follow(gain_meter_curr, goal, fxp12::ONE / 2);

    g.drawString(fonts::font5, x0, y0, "AMP");

    int32_t peakAmp = SHPC_ROUND_DIV(fxp12::log2(sts.rf.adc_amplitude_peak * fxp12::ONE), 12);
    peakAmp = SHPC_CLIP(0, 100, SHPC_ROUND_DIV(peakAmp * 100, fxp12::ONE));

    ampNumber.setNumber(nowMs, peakAmp);
    ampNumber.update(nowMs);
    ampNumber.render(g, x0 + 32 - ampNumber.width, y0);

    render_meter(nowMs, g, 0, y0 + 6, gain_meter_curr);
}

static void render_quarity_meter(uint64_t nowMs, ssd1306::Screen &g, int x0, int y0) {
    qty_meter_curr = fxp12::follow(qty_meter_curr, sts.sync.bit_det_quality, fxp12::ONE / 8);

    //int32_t qty = qty_meter_curr * sts.rf.signal_quarity / jjy::ONE;
    int32_t qty = (qty_meter_curr + sts.rf.signal_quarity) / 2;
    //int32_t qty = sts.rf.signal_quarity;

    g.drawString(fonts::font5, x0, y0, "QTY");
    
    qtyNumber.setNumber(nowMs, qty * 100 / jjy::ONE);
    qtyNumber.update(nowMs);
    qtyNumber.render(g, x0 + 32 - qtyNumber.width, y0);
    
    render_meter(nowMs, g, 0, y0 + 6, qty);
}

static void render_scope(uint64_t nowMs, ssd1306::Screen &g, int x0, int y0, const receiver_status_t &sts) {
    int w = jjy::rx::DifferentialDetector::SCOPE_SIZE;
    const uint32_t *scope = sts.scope;
    for (int x = 0; x < w; x++) {
        uint32_t val = scope[x];
        for (int y = 0; y < 8; y++) {
            val = (val >> 2) | (val & 3);
            val = (val >> 1) | (val & 1);
            if (val & 1) {
                g.set_pixel(x0 + x, y0 + y);
            }
            val >>= 1;
        }

    }
}

static void render_meter(uint64_t nowMs, ssd1306::Screen &g, int x0, int y0, int32_t val) {
    constexpr int32_t A_PERIOD = fxp12::PHASE_PERIOD * 45 / 360;
    constexpr int RADIUS_MAX = 40;
    constexpr int RADIUS_MIN = RADIUS_MAX - 10;

    g.drawBitmap(x0, y0, bmp_meter_frame);
    
    val = FXP_CLIP(0, fxp12::ONE, val);
    int32_t a = (fxp12::PHASE_PERIOD * 3 / 4 - A_PERIOD / 2) + (A_PERIOD * val) / fxp12::ONE;
    int32_t sin = fxp12::fastSin(a);
    int32_t cos = fxp12::fastCos(a);
    int32_t cx = (x0 + 16) * fxp12::ONE;
    int32_t cy = (y0 + RADIUS_MAX) * fxp12::ONE + fxp12::ONE / 2;
    int32_t lx0 = cx + cos * (RADIUS_MAX - 1);
    int32_t ly0 = cy + sin * (RADIUS_MAX - 1);
    int32_t lx1 = cx + cos * RADIUS_MIN;
    int32_t ly1 = cy + sin * RADIUS_MIN;
    g.drawLineF(lx0 - fxp12::ONE / 2, ly0, lx1 - fxp12::ONE / 2, ly1);
    g.drawLineF(lx0 + fxp12::ONE / 2, ly0, lx1 + fxp12::ONE / 2, ly1);
}

//todo: 削除
//static void render_date_time(uint64_t nowMs, ssd1306::Screen &g, const receiver_status_t &sts) { 
//    char s[32];
//    const int stsY = LCD_H - 12 - 6;
//    const int guageX = LCD_W / 2;
//    const int guageW = LCD_W - guageX;
//    //bool empty = true;
//    if (!sts.sync.phase_locked) {
//        g.drawString(fonts::font5, 0, stsY, "PHASE SYNC...");
//        const int guageVal = (LCD_W / 2) * sts.sync.phase_lock_progress / jjy::ONE;
//        g.draw_rect(guageX, stsY, guageW - 1, 4);
//        g.fillRect(guageX, stsY + 1, guageVal, 3);
//    }
//    else if (!sts.dec.synced) {
//        g.drawString(fonts::font5, 0, stsY, "FRAME SYNC...");
//        int pos = nowMs % 1024;
//        if (pos > 512) pos = 1024 - pos;
//        const int guagePos = (LCD_W * 3 / 8) * pos / 512;
//        g.draw_rect(guageX, stsY, guageW - 1, 4);
//        g.fillRect(guageX + guagePos, stsY + 1, (LCD_W / 8), 3);
//    }
//    else if (sts.dec.last_parse_result.flags == jjy::ParseResut::EMPTY) {
//        g.drawString(fonts::font5, 0, stsY, "RECEIVING...");
//        const int guageVal = (LCD_W / 2) * sts.dec.last_bit_index / 60;
//        g.draw_rect(guageX, stsY, guageW - 1, 4);
//        g.fillRect(guageX, stsY + 1, guageVal, 3);
//    }
//    else if (sts.dec.last_parse_result.success()) {
//        //lcd.draw_string(bmpfont::font5, 0, sts_y, "NO ERROR");
//        //goal_date_time = sts.dec.last_date_time;
//        //goal_date_time.second = sts.dec.last_bit_index;
//        //empty = false;
//    }
//    else {
//        sprintf(s, "ERROR CODE = 0x%x", (int)(sts.dec.last_parse_result.flags));
//        g.drawString(fonts::font5, 0, stsY, s);
//    }
// 
//    //static int tmp_t = 0;
//    //if ((tmp_t++) % 3 == 0) {
//    //    disp_date_time.year = (disp_date_time.year / 100 < goal_date_time.year / 100) ? disp_date_time.year + 100 : goal_date_time.year;
//    //    disp_date_time.year = (disp_date_time.year < goal_date_time.year) ? disp_date_time.year + 1 : goal_date_time.year;
//    //    disp_date_time.month = (disp_date_time.month < goal_date_time.month) ? disp_date_time.month + 1 : goal_date_time.month;
//    //    disp_date_time.day = (disp_date_time.day < goal_date_time.day) ? disp_date_time.day + 1 : goal_date_time.day;
//    //    disp_date_time.hour = (disp_date_time.hour < goal_date_time.hour) ? disp_date_time.hour + 1 : goal_date_time.hour;
//    //    disp_date_time.minute = (disp_date_time.minute < goal_date_time.minute) ? disp_date_time.minute + 1 : goal_date_time.minute;
//    //    disp_date_time.second = (disp_date_time.second < goal_date_time.second) ? disp_date_time.second + 1 : goal_date_time.second;
//    //}
//
//    //if (empty) {
//    //    lcd.drawString(fonts::font12, 0, LCD_H - 12, "----/--/-- --:--:--");
//    //}
//    //else {
//    //    sprintf(s, "%04d/%02d/%02d %02d:%02d:%02d",
//    //        disp_date_time.year,
//    //        disp_date_time.month - jjy::MONTH_OFFSET + 1,
//    //        disp_date_time.day - jjy::DAY_OFFSET + 1,
//    //        disp_date_time.hour,
//    //        disp_date_time.minute,
//    //        sts.dec.last_bit_index);
//    //    lcd.drawString(fonts::font12, 0, LCD_H - 12, s);
//    //}
//
//}

}
