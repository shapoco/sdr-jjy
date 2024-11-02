#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include "core0.hpp"
#include "core1.hpp"

#include "atomic.hpp"
#include "jjy/jjy.hpp"
#include "fixed12.hpp"

#include "ssd1309spi.hpp"
#include "bmpfont/bmpfont.hpp"

using pen_t = ssd1309spi::pen_t;

extern atomic<jjy::rx::rf_status_t> glb_rf_status;
extern atomic<jjy::rx::sync_status_t> glb_sync_status;

static constexpr int LCD_W = 128;
static constexpr int LCD_H = 64;

static constexpr int LCD_SPI_INDEX = 0;
static constexpr uint32_t LCD_SPI_FREQ = 1e6;
static constexpr int PIN_LCD_CS_N = 17;
static constexpr int PIN_LCD_SCLK = 18;
static constexpr int PIN_LCD_MOSI = 19;
static constexpr int PIN_LCD_RES_N = 20;
static constexpr int PIN_LCD_DC = 21;

ssd1309spi::Lcd<LCD_W, LCD_H, LCD_SPI_INDEX, LCD_SPI_FREQ, PIN_LCD_RES_N, PIN_LCD_CS_N, PIN_LCD_DC, PIN_LCD_SCLK, PIN_LCD_MOSI> lcd;

static constexpr int FXP_PREC = 12;

static int32_t sin_table[jjy::PHASE_PERIOD];
static int32_t cos_table[jjy::PHASE_PERIOD];

static constexpr int WAVEFORM_PERIOD_MS = 1000;
static constexpr int RADER_PERIOD = 1 << 7;
static constexpr int RADAR_RADIUS = 16;
static float rader_max[RADER_PERIOD];
static float rader_min[RADER_PERIOD];

static void paint_rader(uint32_t t_now_ms);

static float gain = 1;

static int32_t curr_phase = 0;
static int last_wfm_pos = -1;
jjy::rx::rf_status_t rf_sts;
jjy::rx::sync_status_t sync_sts;

void core1_init() {
    for (int i = 0; i < jjy::PHASE_PERIOD; i++) {
        sin_table[i] = round(sin(i * 2 * M_PI / jjy::PHASE_PERIOD) * (1 << FXP_PREC));
        cos_table[i] = round(cos(i * 2 * M_PI / jjy::PHASE_PERIOD) * (1 << FXP_PREC));
    }
    lcd.init();
}

void core1_main() {
    uint32_t t_now_ms = to_ms_since_boot(get_absolute_time());
    uint32_t t_next_paint_ms = t_now_ms;
    uint32_t t_next_sync_ms = t_now_ms;

    rf_sts = glb_rf_status.load();
    sync_sts = glb_sync_status.load();

    while (true) {
        uint32_t t_now_ms = to_ms_since_boot(get_absolute_time());

        if (t_now_ms >= t_next_sync_ms) {
            t_next_sync_ms = t_now_ms + 5;
            rf_sts = glb_rf_status.load();

            int curr_wfm_pos = (t_now_ms % WAVEFORM_PERIOD_MS) * RADER_PERIOD / WAVEFORM_PERIOD_MS;
            if (curr_wfm_pos != last_wfm_pos) {
                rader_max[curr_wfm_pos] = -9999;
                rader_min[curr_wfm_pos] = 9999;
            }

            float sig = rf_sts.signal_level() * RADAR_RADIUS;
            rader_max[curr_wfm_pos] = JJY_MAX(rader_max[curr_wfm_pos], sig);
            rader_min[curr_wfm_pos] = JJY_MIN(rader_min[curr_wfm_pos], sig);
            last_wfm_pos = curr_wfm_pos;
        }

        if (t_now_ms >= t_next_paint_ms) {
            t_next_paint_ms = t_now_ms + 20;
            lcd.clear();

            sync_sts = glb_sync_status.load();

            int32_t phase_diff = jjy::calc_phase_diff(sync_sts.phase, curr_phase);
            curr_phase = jjy::phase_add(curr_phase, phase_diff / (1 << 4));

            paint_rader(t_now_ms);

            {
                int x = 0;
                int y = 32;
                const bmpfont::Font &font = bmpfont::font16;
                if (!sync_sts.phase_locked) {
                    lcd.draw_string(font, x, y, "Phase Sync...");
                    y += font.height - 2;
                    lcd.fill_rect(x, y, (LCD_W / 2) * sync_sts.phase_lock_progress / jjy::ONE, 1);
                }
                else if (!sync_sts.bit_det_ok) {
                    lcd.draw_string(font, x, y, "Pulse Check...");
                    y += font.height - 2;
                    lcd.fill_rect(x, y, (LCD_W / 2) + (LCD_W / 2) * sync_sts.bit_det_progress / jjy::ONE, 1);
                }
            }

            lcd.commit();
        }
        lcd.service();
    }
}

static void paint_rader(uint32_t t_now_ms) {
    const int cx = RADAR_RADIUS;
    const int cy = RADAR_RADIUS;

    // RADAR描画
    for (int pos = 0; pos < RADER_PERIOD; pos++) {
        int max = JJY_MAX(rader_max[pos], rader_min[(pos + RADER_PERIOD - 1) % RADER_PERIOD]);
        int min = JJY_MIN(rader_min[pos], rader_max[(pos + RADER_PERIOD - 1) % RADER_PERIOD]);
        int r1 = RADAR_RADIUS / 2 + JJY_CLIP(1, RADAR_RADIUS, max * gain) / 2;
        int r0 = RADAR_RADIUS / 2 + JJY_CLIP(1, RADAR_RADIUS, min * gain) / 2;
        int32_t a = pos * jjy::PHASE_PERIOD / RADER_PERIOD;
        a = jjy::phase_add(a, -curr_phase);
        int x0 = cx + sin_table[a] * r0 / (1 << FXP_PREC);
        int y0 = cy - cos_table[a] * r0 / (1 << FXP_PREC);
        int x1 = cx + sin_table[a] * r1 / (1 << FXP_PREC);
        int y1 = cy - cos_table[a] * r1 / (1 << FXP_PREC);
        lcd.draw_line(x0, y0, x1, y1, pen_t::WHITE);
        lcd.set_pixel(x1, y1, pen_t::WHITE);
    }

    // カーソル描画
    {
        int32_t a = last_wfm_pos * jjy::PHASE_PERIOD / RADER_PERIOD;
        a = jjy::phase_add(a, -curr_phase);
        int x1 = sin_table[a] * RADAR_RADIUS / (1 << FXP_PREC);
        int y1 = cos_table[a] * RADAR_RADIUS / (1 << FXP_PREC);
        lcd.draw_line(cx + x1 / 2, cy - y1 / 2, cx + x1, cy - y1, pen_t::WHITE);
    }

    // 信号レベル描画
    {
        uint32_t range = rf_sts.det_signal_peak - rf_sts.det_signal_base;
        uint32_t val = rf_sts.det_level_raw - rf_sts.det_signal_base;
        int w = JJY_CLIP(0, range, val) * (RADAR_RADIUS * fxp12::ONE / 2) / range;
        lcd.fill_ellipse_f(cx * fxp12::ONE - w / 2, cy * fxp12::ONE - w / 2, w, w, pen_t::WHITE);
    }
}
