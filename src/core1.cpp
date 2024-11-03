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
    LazyTimer<uint32_t, 5> waveform_update_timer;
    LazyTimer<uint32_t, 20> render_timer;

    int32_t gain_meter_scale = fxp12::ONE;
    int32_t gain_meter_curr = 0;
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
                const int y0 = 0;
                
                {
                    if (sts.rf.agc_gain > gain_meter_scale * 3 / 2) {
                        gain_meter_scale *= 2;
                    }
                    else if (sts.rf.agc_gain < gain_meter_scale * 3 / 4) {
                        gain_meter_scale /= 2;
                    }
                    //int32_t diff = (sts.rf.agc_gain - gain_meter_scale) / (1 << 4);
                    //gain_meter_scale += diff;
                }

                {
                    int32_t goal = sts.rf.adc_amplitude_raw * gain_meter_scale / (jjy::rx::Agc::GOAL_AMPLITUDE * 5 / 4);
                    int32_t diff = (goal - gain_meter_curr) / (1 << 1);
                    gain_meter_curr += diff;
                }
                amp_meter.render(t_now_ms, 0, y0 + 6, lcd, gain_meter_curr);

                const int sx0 = 0;
                lcd.fill_rect(sx0 - 1, y0, 17, 6, pen_t::BLACK);
                lcd.draw_string(bmpfont::font5, sx0, y0, "AMP");

                const int zx0 = 22;
                char s[8];
                if (gain_meter_scale >= jjy::ONE) {
                    sprintf(s, "x%1d", gain_meter_scale / jjy::ONE);
                }
                else {
                    sprintf(s, "/%1d", jjy::ONE / gain_meter_scale);
                }
                //sprintf(s, "x%3.1f", (float)gain_meter_scale / jjy::ONE);
                //lcd.fill_rect(zx0 - 1, 0, 32 - zx0, 6, pen_t::BLACK);
                lcd.draw_string(bmpfont::font5, zx0, y0, s);
            }

            {
                const int y0 = 18;
                int32_t diff = (sts.sync.bit_det_quality - qty_meter_curr) / (1 << 3);
                qty_meter_curr += diff;
                int32_t qty = qty_meter_curr * sts.rf.signal_quarity / jjy::ONE;
                //int32_t qty = (qty_meter_curr + sts.rf.signal_quarity) / 2;
                qty_meter.render(t_now_ms, 0, y0 + 6, lcd, qty);

                const int sx0 = 0;
                //lcd.fill_rect(sx0 - 1, y0, 17, 6, pen_t::BLACK);
                lcd.draw_string(bmpfont::font5, sx0, y0, "QTY");
                
                const int zx0 = 22;
                char s[8];
                sprintf(s, "%d", qty * 100 / jjy::ONE);
                //sprintf(s, "x%3.1f", (float)gain_meter_scale / jjy::ONE);
                //lcd.fill_rect(zx0 - 1, 0, 32 - zx0, 6, pen_t::BLACK);
                lcd.draw_string(bmpfont::font5, zx0, y0, s);
            }

            {
                const int x0 = 40;
                const int y0 = 0;
                //lcd.fill_rect(sx0 - 1, y0, 17, 6, pen_t::BLACK);
                lcd.draw_string(bmpfont::font5, x0 + 4, y0, "PHASE");
                rader.render(t_now_ms, x0 + Rader::RADIUS, y0 + 6 + Rader::RADIUS, lcd, sts);
            }
            render_sync_status(t_now_ms);
            lcd.commit();
        }

        lcd.service();
    }
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