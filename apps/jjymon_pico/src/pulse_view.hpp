#pragma once

#include <stdio.h>
#include <string.h>

#include "shapoco/jjy/jjy.hpp"
#include "shapoco/math_utils.hpp"
#include "shapoco/fixed12.hpp"
#include "shapoco/graphics/graphics.hpp"
#include "shapoco/ssd1306/pico/spi_lcd.hpp"

#include "jjymon.hpp"
#include "ui.hpp"
#include "images.hpp"
#include "fonts.hpp"
#include "rotary_counter.hpp"

namespace shapoco::jjymon {

using namespace shapoco::graphics;
using pen_t = shapoco::ssd1306::pen_t;

class PulseView {
public:
    static constexpr int WIDTH = 38;
    static constexpr int HEIGHT = 44;

    static constexpr int TITLE_HEIGHT = 6;

    static constexpr int LARGE_LINE_HEIGHT = 8;
    static constexpr int SMALL_LINE_HEIGHT = 3;

    static constexpr int NUM_LINES = 2 + SHPC_CEIL_DIV(HEIGHT - TITLE_HEIGHT - LARGE_LINE_HEIGHT - 1, SMALL_LINE_HEIGHT + 1);
    static constexpr int BUFF_SIZE = NUM_LINES * WIDTH;
    static constexpr int BUFF_PERIOD_MS = NUM_LINES * 1000;

private:
    uint8_t buff[BUFF_SIZE];
    int32_t goalRdPtrFxp = 0;
    int32_t dispRdPtrFxp = 0;
    int wrPtr = 0;
    int32_t goalPhase = 0;
    int32_t currPhase = 0;
    bool beatDetected = false;

public:
    PulseView() { }

    void update(uint64_t t_ms, jjymon::receiver_status_t &sts) {
        int newWrPhaseMs = sts.rf.timestamp_ms % BUFF_PERIOD_MS;
        int newWrPtr = newWrPhaseMs * BUFF_SIZE / BUFF_PERIOD_MS;
        if (newWrPtr != wrPtr) {
            wrPtr = newWrPtr;
            int32_t val = SHPC_ROUND_DIV((sts.rf.det_anl_out_norm) * 255, fxp12::ONE);
            buff[wrPtr] = SHPC_CLIP(0, 255, val);
        }
        //if (fxp12::phaseDiff(wrPtr, rdPtr, BUFF_SIZE) >= WIDTH) {
        //    rdPtr = fxp12::phaseAdd(rdPtr, WIDTH, BUFF_SIZE);
        //}
        int localPhaseMs = (sts.sync.phase_cursor - sts.sync.phase_offset) * 1000 / jjy::PHASE_PERIOD;
        int rdPhaseMs = fxp12::phaseNorm(newWrPhaseMs - localPhaseMs, BUFF_PERIOD_MS);
        int rdPtr = rdPhaseMs * BUFF_SIZE / BUFF_PERIOD_MS;
        if (fxp12::phaseDiff(wrPtr, rdPtr, BUFF_SIZE) < 0) {
            rdPtr = fxp12::phaseAdd(rdPtr, -WIDTH, BUFF_SIZE);
        }
        goalRdPtrFxp = rdPtr * fxp12::ONE;
        if (SHPC_ABS(fxp12::phaseDiff(goalRdPtrFxp, dispRdPtrFxp, BUFF_SIZE * fxp12::ONE) - WIDTH * fxp12::ONE) < WIDTH * fxp12::ONE / 10) {
            dispRdPtrFxp = fxp12::phaseAdd(dispRdPtrFxp, WIDTH * fxp12::ONE, BUFF_SIZE * fxp12::ONE);
        }
        else {
            dispRdPtrFxp = fxp12::phaseFollow(dispRdPtrFxp, goalRdPtrFxp, fxp12::ONE / 32, BUFF_SIZE * fxp12::ONE);
        }

        beatDetected = sts.rf.beat_detected;
    }

    void render(ssd1306::Screen &g, int x0, int y0) {
        g.drawString(fonts::font5, x0, y0, "PULSE");

        g.setClipRect(x0, y0 + TITLE_HEIGHT, WIDTH, HEIGHT - TITLE_HEIGHT);
        int y = y0 + HEIGHT - LARGE_LINE_HEIGHT;
        int rdPtr = dispRdPtrFxp / fxp12::ONE;
        int yOffset = SMALL_LINE_HEIGHT - SHPC_CLIP(0, SMALL_LINE_HEIGHT * 2, fxp12::phaseDiff(wrPtr, rdPtr, BUFF_SIZE)) / 2;
        int rp0 = rdPtr;
        for (int iline = 0; iline < NUM_LINES; iline++) {
            int rp = iline == 0 ? fxp12::phaseAdd(wrPtr, -(WIDTH - 1), BUFF_SIZE) : rp0;
            for (int x = 0; x < WIDTH; x++) {
                int lineY = iline == 0 ? y : y + yOffset;
                int lineH = ((iline == 0) ? LARGE_LINE_HEIGHT : SMALL_LINE_HEIGHT) - 1;
                int wavH = 1 + SHPC_ROUND_DIV((int)buff[rp] * lineH, 255);
                g.fillRect(x0 + x, lineY + lineH - wavH, 1, wavH);
                if (rp == wrPtr) break;
                rp = (rp + 1 < BUFF_SIZE) ? (rp + 1) : 0;
            }
            rp0 = fxp12::phaseAdd(rp0, -WIDTH, BUFF_SIZE);
            y -= SMALL_LINE_HEIGHT + 1;
        }
        g.clearClipRect();

        // うなり検出状態
        if (beatDetected) {
            g.draw_bitmap(x0 + WIDTH - bmp_icon_beat[0], y0, bmp_icon_beat);
        }
    }

};

}
