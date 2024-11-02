#ifndef RADER_HPP
#define RADER_HPP

#include "jjymon.hpp"
#include "jjy/jjy.hpp"
#include "fixed12.hpp"
#include "ssd1309spi.hpp"
#include "images/images.hpp"

using pen_t = ssd1309spi::pen_t;

class Rader {
private:
    static constexpr int WAVEFORM_PERIOD_MS = 1000;
    static constexpr int RADER_PERIOD = 1 << 7;
    static constexpr int RADAR_RADIUS = 16;

    float rader_max[RADER_PERIOD];
    float rader_min[RADER_PERIOD];

    int32_t disp_phase_offset = 0;
    int cursor_phase = -1;

public:
    void update(uint32_t t_now_ms, const receiver_status_t &sts) {
        int new_phase = (t_now_ms % WAVEFORM_PERIOD_MS) * RADER_PERIOD / WAVEFORM_PERIOD_MS;
        if (new_phase != cursor_phase) {
            rader_max[new_phase] = -9999;
            rader_min[new_phase] = 9999;
        }

        float sig = sts.rf.signal_level() * RADAR_RADIUS;
        rader_max[new_phase] = JJY_MAX(rader_max[new_phase], sig);
        rader_min[new_phase] = JJY_MIN(rader_min[new_phase], sig);
        cursor_phase = new_phase;
    }

    void render(uint32_t t_now_ms, JjyLcd &lcd, const receiver_status_t &sts) {
        int32_t phase_diff = jjy::calc_phase_diff(sts.sync.phase, disp_phase_offset);
        disp_phase_offset = jjy::phase_add(disp_phase_offset, phase_diff / (1 << 4));

        const int cx = RADAR_RADIUS;
        const int cy = RADAR_RADIUS;

        // RADAR描画
        for (int pos = 0; pos < RADER_PERIOD; pos++) {
            int max = JJY_MAX(rader_max[pos], rader_min[(pos + RADER_PERIOD - 1) % RADER_PERIOD]);
            int min = JJY_MIN(rader_min[pos], rader_max[(pos + RADER_PERIOD - 1) % RADER_PERIOD]);
            int r1 = RADAR_RADIUS / 2 + JJY_CLIP(1, RADAR_RADIUS, max) / 2;
            int r0 = RADAR_RADIUS / 2 + JJY_CLIP(1, RADAR_RADIUS, min) / 2;
            int32_t a = pos * jjy::PHASE_PERIOD / RADER_PERIOD;
            a = jjy::phase_add(a, -disp_phase_offset);
            a = a * fxp12::ANGLE_PERIOD / jjy::PHASE_PERIOD;
            int32_t sin = fxp12::sin(a);
            int32_t cos = fxp12::cos(a);
            int x0 = cx + sin * r0 / fxp12::ONE;
            int y0 = cy - cos * r0 / fxp12::ONE;
            int x1 = cx + sin * r1 / fxp12::ONE;
            int y1 = cy - cos * r1 / fxp12::ONE;
            lcd.draw_line(x0, y0, x1, y1, pen_t::WHITE);
            lcd.set_pixel(x1, y1, pen_t::WHITE);
        }

        // カーソル描画
        {
            int32_t a = cursor_phase * jjy::PHASE_PERIOD / RADER_PERIOD;
            a = jjy::phase_add(a, -disp_phase_offset) * fxp12::ANGLE_PERIOD / jjy::PHASE_PERIOD;
            int x1 = fxp12::sin(a) * RADAR_RADIUS / fxp12::ONE;
            int y1 = -fxp12::cos(a) * RADAR_RADIUS / fxp12::ONE;
            lcd.draw_line(cx + x1 / 2, cy + y1 / 2, cx + x1, cy + y1, pen_t::WHITE);
        }

        // 信号レベル描画
        {
            uint32_t range = sts.rf.det_signal_peak - sts.rf.det_signal_base;
            uint32_t val = sts.rf.det_level_raw - sts.rf.det_signal_base;
            int w = JJY_CLIP(0, range, val) * (RADAR_RADIUS * fxp12::ONE / 2) / range;
            lcd.fill_ellipse_f(cx * fxp12::ONE - w / 2, cy * fxp12::ONE - w / 2, w, w, pen_t::WHITE);
        }
    }

};

#endif
