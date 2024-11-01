#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include "core0.hpp"
#include "core1.hpp"

#include "atomic.hpp"
#include "jjy/jjy.hpp"

#include "ssd1309spi.hpp"
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

static constexpr int WAVEFORM_PERIOD_MS = 1000;
static constexpr int WAVEFORM_W = 100;
static constexpr int WAVEFORM_H = 8;
static float waveform_max[WAVEFORM_W];
static float waveform_min[WAVEFORM_W];

void core1_init() {
    lcd.init();
}

void core1_main() {
    uint32_t t_now_ms = to_ms_since_boot(get_absolute_time());
    uint32_t t_next_paint_ms = t_now_ms;
    uint32_t t_next_sync_ms = t_now_ms;
    jjy::rx::rf_status_t rf_sts;
    jjy::rx::sync_status_t sync_sts;
    int last_wfm_pos = -1;
    float peak = 0;
    float gain = 1;
    while (true) {
        uint32_t t_now_ms = to_ms_since_boot(get_absolute_time());

        if (t_now_ms >= t_next_sync_ms) {
            t_next_sync_ms = t_now_ms + 5;
            rf_sts = glb_rf_status.load();

            int curr_wfm_pos = (t_now_ms % WAVEFORM_PERIOD_MS) * WAVEFORM_W / WAVEFORM_PERIOD_MS;
            if (curr_wfm_pos != last_wfm_pos) {
                waveform_max[curr_wfm_pos] = -9999;
                waveform_min[curr_wfm_pos] = 9999;
            }

            float sig = rf_sts.signal_level() * WAVEFORM_H;
            waveform_max[curr_wfm_pos] = JJY_MAX(waveform_max[curr_wfm_pos], sig);
            waveform_min[curr_wfm_pos] = JJY_MIN(waveform_min[curr_wfm_pos], sig);
            last_wfm_pos = curr_wfm_pos;

            //peak = 1;
            //for (int pos = 0; pos < WAVEFORM_W; pos++) {
            //    //uint32_t abs_peak = JJY_MAX(JJY_ABS(waveform_max[pos]), JJY_ABS(waveform_min[pos]));
            //    //peak = JJY_MAX(peak, abs_peak);
            //    peak = JJY_MAX(peak, waveform_max[pos]);
            //}
            //
            ////gain = ((WAVEFORM_H / 2) << jjy::PREC) / peak;
            //gain = (float)WAVEFORM_H / peak;
        }


        if (t_now_ms >= t_next_paint_ms) {
            t_next_paint_ms = t_now_ms + 20;
            lcd.clear();

            int sync_y = LCD_H - 8 - WAVEFORM_H - 1;
            sync_sts = glb_sync_status.load();
            for (int i = 0; i < jjy::rx::NUM_PHASE_CANDS; i++) {
                jjy::rx::phase_cand_t &cand = sync_sts.phase_cands[i];
                if (!cand.valid) continue;
                int x = cand.phase * WAVEFORM_W / ((WAVEFORM_PERIOD_MS / 1000) * jjy::rx::PHASE_PERIOD);
                int h = cand.weight;
                lcd.fill_rect(x, sync_y - h, 1, h, pen_t::WHITE);    
            }

            int wfm_y = LCD_H - 8;
            lcd.fill_rect(0, 0, peak / 2, 2, pen_t::WHITE);
            lcd.fill_rect(0, 3, rf_sts.adc_level, 2, pen_t::WHITE);
            lcd.fill_rect(0, 6, JJY_ABS(rf_sts.adc_min) / 16, 2, pen_t::WHITE);
            lcd.fill_rect(0, 9, JJY_ABS(rf_sts.adc_max) / 16, 2, pen_t::WHITE);
            lcd.fill_rect(0, 12, rf_sts.det_threshold / 4, 2, pen_t::WHITE);
            for (int pos = 0; pos < WAVEFORM_W; pos++) {
                int max = JJY_CLIP(1, WAVEFORM_H, waveform_max[pos] * gain);
                int min = JJY_CLIP(1, WAVEFORM_H, waveform_min[pos] * gain);
                lcd.fill_rect(pos, wfm_y - max, 1, 1 + JJY_MAX(0, max - min), pen_t::WHITE);
            }
            lcd.fill_rect(last_wfm_pos, wfm_y - WAVEFORM_H, 1, WAVEFORM_H, pen_t::WHITE);
            lcd.commit();
        }
        lcd.service();
    }
}
