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


extern atomic<receiver_status_t> glb_receiver_status;
static receiver_status_t sts;

static JjyLcd lcd;
using pen_t = ssd1309spi::pen_t;

static Rader rader;
static Meter amp_meter;
static Meter qty_meter;

static void render_guages(uint32_t t_now_ms);
static void render_guage(int x, int y, int w, int h, int val);
static void render_meter(int x, int y, int32_t val);
static void render_sync_status(uint32_t t_now_ms);

void core1_init() {
    lcd.init();
}

void core1_main() {
    uint32_t t_now_ms = to_ms_since_boot(get_absolute_time());
    uint32_t t_next_paint_ms = t_now_ms;
    uint32_t t_next_sync_ms = t_now_ms;
    int32_t gain_meter_scale = fxp12::ONE;
    int32_t qty_meter_curr = 0;

    sts = glb_receiver_status.load();

    while (true) {
        uint32_t t_now_ms = to_ms_since_boot(get_absolute_time());

        if (t_now_ms >= t_next_sync_ms) {
            t_next_sync_ms = t_now_ms + 5;
            sts = glb_receiver_status.load();
            rader.update(t_now_ms, sts);
        }

        if (t_now_ms >= t_next_paint_ms) {
            t_next_paint_ms = t_now_ms + 20;

            lcd.clear();
            //render_guages(t_now_ms);
            {
                if (sts.rf.agc_gain > gain_meter_scale * 3 / 2) {
                    gain_meter_scale *= 2;
                }
                else if (sts.rf.agc_gain < gain_meter_scale * 3 / 4) {
                    gain_meter_scale /= 2;
                }

                //int32_t val = sts.rf.adc_amplitude_raw * fxp12::ONE / jjy::rx::Agc::GOAL_AMPLITUDE;
                int32_t val = sts.rf.adc_amplitude_raw * gain_meter_scale / jjy::rx::Agc::GOAL_AMPLITUDE * 3 / 4;
                amp_meter.render(t_now_ms, 0, 0, lcd, val);
            }

            {
                int32_t diff = (sts.sync.bit_det_quality - qty_meter_curr) / (1 << 3);
                qty_meter_curr += diff;
                qty_meter.render(t_now_ms, 0, 16, lcd, qty_meter_curr);
            }

            rader.render(t_now_ms, 32 + Rader::RADIUS, Rader::RADIUS, lcd, sts);
            render_sync_status(t_now_ms);
            lcd.commit();
        }

        lcd.service();
    }
}

static void render_guages(uint32_t t_now_ms) {
    int x = 0, y = 0;
    int w = Rader::DIAMETER - 2, h = 5;
    int w_inner = w - 2;
    {
        int max = fxp12::log2(jjy::rx::Agc::GAIN_MAX);
        int val = fxp12::log2(sts.rf.adc_amplitude_raw * fxp12::ONE) * w_inner / max / 2;
        render_guage(x, y, w - 1, h - 1, val);
        y += h + 1;
    }
}

static void render_guage(int x, int y, int w, int h, int val) {
    lcd.draw_rect(x, y, w, h);
    lcd.fill_rect(x + 1, y + 1, FXP_CLIP(0, w - 1, val), h - 1);
}

static void render_meters(uint32_t t_now_ms) {

}

static void render_meter(int x, int y, int32_t val) {

}

static void render_sync_status(uint32_t t_now_ms) {
    int x = 0;
    int y = 32;
    const bmpfont::Font &font = bmpfont::font16;
    if (!sts.sync.phase_locked) {
        lcd.draw_string(font, x, y, "Phase Sync...");
        y += font.height - 2;
        lcd.fill_rect(x, y, (LCD_W / 2) * sts.sync.phase_lock_progress / jjy::ONE, 1);
    }
    else if (!sts.sync.bit_det_ok) {
        lcd.draw_string(font, x, y, "Pulse Check...");
        y += font.height - 2;
        lcd.fill_rect(x, y, (LCD_W / 2) + (LCD_W / 2) * sts.sync.bit_det_progress / jjy::ONE, 1);
    }
}