#ifndef RADER_HPP
#define RADER_HPP

#include "jjymon.hpp"
#include "jjy/jjy.hpp"
#include "fixed12.hpp"
#include "ssd1309spi.hpp"
#include "images/images.hpp"

using pen_t = ssd1309spi::pen_t;

class Rader {
public:
    static constexpr int RADIUS = 16;
    static constexpr int DIAMETER = RADIUS * 2;

private:
    static constexpr int WAVEFORM_PERIOD = 1 << 7;

    int32_t waveform_max[WAVEFORM_PERIOD];
    int32_t waveform_min[WAVEFORM_PERIOD];

    int32_t disp_phase_offset = 0;
    int cursor_phase = 0;

public:
    void update(uint32_t t_now_ms, const receiver_status_t &sts) {
        int new_phase = (t_now_ms % 1000) * WAVEFORM_PERIOD / 1000;
        if (new_phase != cursor_phase) {
            waveform_max[new_phase] = -9999 * fxp12::ONE;
            waveform_min[new_phase] = 9999 * fxp12::ONE;
        }

        int32_t sig = sts.rf.det_anl_out_norm * RADIUS / jjy::ONE;
        waveform_max[new_phase] = JJY_MAX(waveform_max[new_phase], sig);
        waveform_min[new_phase] = JJY_MIN(waveform_min[new_phase], sig);
        cursor_phase = new_phase;
    }

    void render(uint32_t t_now_ms, int cx, int cy, JjyLcd &lcd, const receiver_status_t &sts) {
        int32_t phase_diff = jjy::calc_phase_diff(sts.sync.phase, disp_phase_offset);
        disp_phase_offset = jjy::phase_add(disp_phase_offset, phase_diff / (1 << 4));

        int32_t cxf = cx * fxp12::ONE;
        int32_t cyf = cy * fxp12::ONE;

        // RADAR描画
        for (int pos = 0; pos < WAVEFORM_PERIOD; pos++) {
            int max = JJY_MAX(waveform_max[pos], waveform_min[(pos + WAVEFORM_PERIOD - 1) % WAVEFORM_PERIOD]);
            int min = JJY_MIN(waveform_min[pos], waveform_max[(pos + WAVEFORM_PERIOD - 1) % WAVEFORM_PERIOD]);
            int r1 = RADIUS / 2 + JJY_CLIP(1, RADIUS, max) / 2;
            int r0 = RADIUS / 2 + JJY_CLIP(1, RADIUS, min) / 2;
            int32_t a = pos * jjy::PHASE_PERIOD / WAVEFORM_PERIOD;
            a = jjy::phase_add(a, -disp_phase_offset);
            a = a * fxp12::ANGLE_PERIOD / jjy::PHASE_PERIOD - fxp12::ANGLE_PERIOD / 4;
            int32_t sin = fxp12::sin(a);
            int32_t cos = fxp12::cos(a);
            int32_t x0 = cxf + cos * r0;
            int32_t y0 = cyf + sin * r0;
            int32_t x1 = cxf + cos * r1;
            int32_t y1 = cyf + sin * r1;
            lcd.draw_line_f(x0, y0, x1, y1, pen_t::WHITE);
            lcd.set_pixel(fxp12::to_int(x1), fxp12::to_int(y1), pen_t::WHITE);
        }

        // カーソル描画
        {
            int32_t a = cursor_phase * jjy::PHASE_PERIOD / WAVEFORM_PERIOD;
            a = jjy::phase_add(a, -disp_phase_offset) * fxp12::ANGLE_PERIOD / jjy::PHASE_PERIOD - fxp12::ANGLE_PERIOD / 4;
            int32_t x = fxp12::cos(a) * RADIUS;
            int32_t y = fxp12::sin(a) * RADIUS;
            lcd.draw_line_f(cxf + x / 2, cyf + y / 2, cxf + x, cyf + y, pen_t::WHITE);
        }

        // 信号レベル描画
        {
            uint32_t range = sts.rf.det_anl_out_peak - sts.rf.det_anl_out_base;
            uint32_t val = sts.rf.det_anl_out_raw - sts.rf.det_anl_out_base;
            int w = JJY_CLIP(0, range, val) * (RADIUS * fxp12::ONE / 2) / range;
            lcd.fill_ellipse_f(cx * fxp12::ONE - w / 2, cy * fxp12::ONE - w / 2, w, w, pen_t::WHITE);
        }
    }

};

#endif
