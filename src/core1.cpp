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

#include "lazy_timer.hpp"

extern atomic<receiver_status_t> glb_receiver_status;
static receiver_status_t sts;

static constexpr int FPS = 50;

static JjyLcd lcd;
using pen_t = ssd1309spi::pen_t;

static Rader rader;
static Meter amp_meter;
static Meter qty_meter;

static void render_sync_status(uint32_t t_now_ms);

void core1_init() {
    lcd.init();
}

void core1_main() {
    LazyTimer<uint32_t> waveform_update_timer(5);
    LazyTimer<uint32_t> render_timer(20);

    int32_t gain_meter_scale = fxp12::ONE;
    int32_t qty_meter_curr = 0;

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
            //render_guages(t_now_ms);
            {
                if (sts.rf.agc_gain > gain_meter_scale * 3 / 2) {
                    gain_meter_scale *= 2;
                }
                else if (sts.rf.agc_gain < gain_meter_scale * 3 / 4) {
                    gain_meter_scale /= 2;
                }

                int32_t val = sts.rf.adc_amplitude_raw * gain_meter_scale / (jjy::rx::Agc::GOAL_AMPLITUDE * 3 / 2);
                amp_meter.render(t_now_ms, 0, 0, lcd, val);

                lcd.fill_rect(7, 8, 17, 6, pen_t::BLACK);
                lcd.draw_string(bmpfont::font5, 8, 9, "AMP");

                char s[16];
                if (gain_meter_scale >= jjy::ONE) {
                    sprintf(s, "x%1d", gain_meter_scale / jjy::ONE);
                }
                else {
                    sprintf(s, "/%1d", jjy::ONE / gain_meter_scale);
                }
                lcd.fill_rect(23, 0, 9, 6, pen_t::BLACK);
                lcd.draw_string(bmpfont::font5, 24, 0, s);
            }

            {
                int32_t diff = (sts.sync.bit_det_quality - qty_meter_curr) / (1 << 3);
                qty_meter_curr += diff;
                qty_meter.render(t_now_ms, 0, 16, lcd, qty_meter_curr);

                lcd.fill_rect(7, 24, 17, 6, pen_t::BLACK);
                lcd.draw_string(bmpfont::font5, 8, 25, "QTY");
            }

            rader.render(t_now_ms, 32 + Rader::RADIUS, Rader::RADIUS, lcd, sts);
            render_sync_status(t_now_ms);
            lcd.commit();
        }

        lcd.service();
    }
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