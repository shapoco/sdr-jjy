#pragma once

#include "shapoco/fixed12.hpp"
#include "shapoco/jjy/jjy.hpp"
#include "shapoco/graphics/graphics.hpp"
#include "shapoco/pico/ssd1309spi.hpp"

#include "jjymon.hpp"
#include "ui.hpp"
#include "images.hpp"
#include "fonts.hpp"

namespace shapoco::jjymon {

using namespace ::shapoco::graphics;
using pen_t = ::shapoco::pico::pen_t;

class Rader {
public:
    static constexpr int RADIUS = 16;
    static constexpr int DIAMETER = RADIUS * 2;

    static constexpr int WIDTH = DIAMETER;
    static constexpr int HEIGHT = DIAMETER + 6;

private:
    static constexpr int WAVEFORM_PERIOD = 1 << 7;
    static constexpr int WAVEFORM_BASE = RADIUS / 2;
    static constexpr int WAVEFORM_RANGE = RADIUS - WAVEFORM_BASE;

    int32_t waveform[WAVEFORM_PERIOD];

    int32_t disp_phase_offset = 0;
    int last_phase = 0;

public:
    void update(uint32_t t_now_ms, const receiver_status_t &sts) {
        int curr_phase = (sts.rf.timestamp_ms % 1000) * WAVEFORM_PERIOD / 1000;
        //int32_t curr_val = FXP_CLIP(0, fxp12::ONE, sts.rf.det_anl_out_norm * fxp12::ONE / jjy::ONE);
        int32_t curr_val = FXP_CLIP(0, fxp12::ONE, (sts.rf.det_anl_out_norm + jjy::ONE * 2 / 10) * fxp12::ONE / (jjy::ONE * 14 / 10));
        //int32_t curr_val = FXP_CLIP(0, fxp12::ONE, (sts.rf.det_anl_out_beat_det + jjy::ONE * 2 / 10) * fxp12::ONE / (jjy::ONE * 14 / 10));

        if (curr_phase != last_phase) {
            int phase_step = (curr_phase + WAVEFORM_PERIOD - last_phase) % WAVEFORM_PERIOD;
            int32_t last_val = waveform[last_phase];
            for (int i = 0; i <= phase_step; i++) {
                int p = (last_phase + i) % WAVEFORM_PERIOD;
                waveform[p] = last_val + (curr_val - last_val) * i / phase_step;
            }
            last_phase = curr_phase;
        }
    }

    void render(uint32_t t_now_ms, int rx0, int ry0, JjyLcd &lcd, const receiver_status_t &sts) {
        int32_t delay_phase = (sts.rf.anti_chat_delay_ms + sts.rf.det_delay_ms) * jjy::PHASE_PERIOD / 1000; // 検波器の遅延
        int32_t goal_phase_offset = jjy::phase_add(sts.sync.phase_offset, -delay_phase);
        jjy::phase_follow(&disp_phase_offset, goal_phase_offset, jjy::ONE / 16);

        int cx = rx0 + RADIUS;
        int cy = ry0 + 6 + RADIUS;
        int cxf = cx * fxp12::ONE;
        int cyf = cy * fxp12::ONE;

        lcd.draw_string(fonts::font5, rx0 + 4, ry0, "PHASE");

        // 波形描画
        int last_x, last_y;
        for (int i = 0; i < WAVEFORM_PERIOD; i++) {
            int p = (last_phase + 1 + i) % WAVEFORM_PERIOD;
            int32_t r = WAVEFORM_BASE * fxp12::ONE + waveform[p] * WAVEFORM_RANGE;
            int32_t a = p * jjy::PHASE_PERIOD / WAVEFORM_PERIOD;
            a = jjy::phase_add(a, -disp_phase_offset);
            a = a * fxp12::ANGLE_PERIOD / jjy::PHASE_PERIOD - fxp12::ANGLE_PERIOD / 4;
            int x = cxf + fxp12::cos(a) * r / fxp12::ONE;
            int y = cyf + fxp12::sin(a) * r / fxp12::ONE;
            if (i > 0) {
                lcd.draw_line_f(last_x, last_y, x, y, pen_t::WHITE);
                lcd.set_pixel(fxp12::to_int(x), fxp12::to_int(y), pen_t::WHITE);
            }
            last_x = x;
            last_y = y;
        }

        // カーソル描画
        {
            int32_t a = last_phase * jjy::PHASE_PERIOD / WAVEFORM_PERIOD;
            a = jjy::phase_add(a, -disp_phase_offset) * fxp12::ANGLE_PERIOD / jjy::PHASE_PERIOD - fxp12::ANGLE_PERIOD / 4;
            int32_t x = fxp12::cos(a) * RADIUS;
            int32_t y = fxp12::sin(a) * RADIUS;
            lcd.draw_line_f(cxf + x / 2, cyf + y / 2, cxf + x, cyf + y, pen_t::WHITE);
        }

        // 信号レベル描画
        {
            constexpr int W_MIN = 2 * fxp12::ONE;
            constexpr int W_RANGE = RADIUS * fxp12::ONE * 3 / 4 - W_MIN;
            //int32_t val = FXP_CLIP(0, fxp12::ONE, sts.rf.det_anl_out_norm * fxp12::ONE / jjy::ONE);
            //int32_t val = FXP_CLIP(0, fxp12::ONE, (sts.rf.det_anl_out_norm + jjy::ONE * 8 / 100) * fxp12::ONE / (jjy::ONE * 116 / 100));
            int32_t val = FXP_CLIP(0, fxp12::ONE, (sts.rf.det_anl_out_beat_det + jjy::ONE * 8 / 100) * fxp12::ONE / (jjy::ONE * 116 / 100));
            int w = W_MIN + val * W_RANGE / fxp12::ONE;
            lcd.fill_ellipse_f(cx * fxp12::ONE - w / 2, cy * fxp12::ONE - w / 2, w, w, pen_t::WHITE);
        }

        // うなり検出状態
        if (sts.rf.beat_detected) {
            lcd.draw_bitmap(rx0 - 6, HEIGHT - 7, bmp_icon_beat);
        }
    }

};

}
