#pragma once

#include <stdio.h>
#include <string.h>

#include "shapoco/jjy/jjy.hpp"
#include "shapoco/shapoco.hpp"
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

    static constexpr int SCOPE_HEIGHT = 8;

    static constexpr int LOG_ENTRY_HEIGHT = 4;
    static constexpr int LOG_DEPTH = 1 + SHPC_CEIL_DIV(HEIGHT - TITLE_HEIGHT - SCOPE_HEIGHT - 1, LOG_ENTRY_HEIGHT + 1);

    static constexpr int RANGE_MS = 1100;
    static constexpr int ANIMATION_DURATION_MS = 200;


    struct LogEntry {
        uint8_t waveform[RANGE_MS];
        bool valid = false;

        void clear(uint64_t t_ms) {
            valid = false;
        }
    };

private:
    bool beatDetected = false;

    using cursor_t = cyclic<int, WIDTH>;
    uint8_t scopeWaveform[WIDTH];
    //int scopeCursor = 0;
    cursor_t scopeCursor = 0;

    int32_t lastPhase = 0;

    using logptr_t = cyclic<int, LOG_DEPTH>;
    LogEntry log[LOG_DEPTH];
    logptr_t logCursor = 0;

    uint64_t tAnimationEndMs = 0;

public:
    PulseView() { }

    void init(uint64_t nowMs) {
        for (int ientry = 0; ientry < LOG_DEPTH; ientry++) {
            log[ientry].clear(nowMs);
        }
        tAnimationEndMs = nowMs;
    }

    void update(uint64_t nowMs, jjymon::receiver_status_t &sts) {
        cursor_t newScopeCursor = (sts.rf.timestamp_ms % RANGE_MS) * WIDTH / RANGE_MS;
        if (newScopeCursor != scopeCursor) {
            scopeCursor = newScopeCursor;
            int32_t val = SHPC_ROUND_DIV((sts.rf.det_anl_out_norm) * 255 + fxp12::ONE / 32, fxp12::ONE * 34 / 32);
            scopeWaveform[scopeCursor] = SHPC_CLIP(0, 255, val);
        }

        int32_t phase = fxp12::phaseNorm(sts.sync.phase_cursor - sts.sync.phase_offset + fxp12::PHASE_PERIOD / 10);
        if (phase < jjy::PHASE_PERIOD / 2 && jjy::PHASE_PERIOD / 2 <= lastPhase) {
            LogEntry &entry = log[logCursor];
            cursor_t rp = newScopeCursor + 1;
            for (int i = 0; i < WIDTH; i++) {
                entry.waveform[i] = scopeWaveform[rp];
                entry.valid = true;
                rp++;
            }
            logCursor++;

            tAnimationEndMs = nowMs + ANIMATION_DURATION_MS;
        }
        lastPhase = phase;

        beatDetected = sts.rf.beat_detected;
    }

    void render(uint64_t nowMs, ssd1306::Screen &g, int x0, int y0) {
        g.drawString(fonts::font5, x0, y0, "PULSE");

        renderWaveform(g, x0, y0 + HEIGHT - SCOPE_HEIGHT, SCOPE_HEIGHT, scopeWaveform, scopeCursor + 1, 255);

        g.setClipRect(x0, y0 + TITLE_HEIGHT, WIDTH, HEIGHT - TITLE_HEIGHT - SCOPE_HEIGHT - 1);
        int yAnimation = (nowMs < tAnimationEndMs) ? ((tAnimationEndMs - nowMs) * LOG_ENTRY_HEIGHT / ANIMATION_DURATION_MS) : 0;
        int y = y0 + HEIGHT - SCOPE_HEIGHT - 1 - LOG_ENTRY_HEIGHT + yAnimation;
        logptr_t ientry = logCursor - 1;
        for (int i = 0; i < LOG_DEPTH; i++) {
            LogEntry &entry = log[ientry];
            if (entry.valid) {
                renderWaveform(g, x0, y, LOG_ENTRY_HEIGHT, entry.waveform, 0, 255);
            }
            y -= LOG_ENTRY_HEIGHT + 1;
            ientry--;
        }
        g.clearClipRect();

        // うなり検出状態
        if (beatDetected) {
            g.drawBitmap(x0 + WIDTH - bmp_icon_beat[0], y0, bmp_icon_beat);
        }
    }

    void renderWaveform(ssd1306::Screen &g, int x0, int y0, int h, const uint8_t *waveform, cursor_t cursor, int peak) {
        int32_t hScale = (h - 1) * fxp12::ONE / peak;
        for (int x = 0; x < WIDTH; x++) {
            int wh = 1 + ((waveform[cursor] * hScale + fxp12::ONE / 2) >> fxp12::PREC);
            g.fillRect(x0 + x, y0 + h - wh, 1, wh);
            cursor++;
        }
    }

};

}
