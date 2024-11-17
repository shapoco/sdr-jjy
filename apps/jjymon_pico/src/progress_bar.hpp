#pragma once

#include <stdio.h>
#include <string.h>

#include "shapoco/jjy/jjy.hpp"
#include "shapoco/shapoco.hpp"
#include "shapoco/fixed12.hpp"
#include "shapoco/graphics/graphics.hpp"
#include "shapoco/ssd1306/pico/spi_lcd.hpp"

#include "ui.hpp"
#include "fonts.hpp"

namespace shapoco::jjymon {

using namespace shapoco::graphics;
using pen_t = shapoco::ssd1306::pen_t;

class ProgressBar {
public:
    static constexpr int ANIMATION_DURATION_MS = 500;
    static constexpr int PADDING = 3;

    const TinyFont &font;
    const int width, height;
    int32_t goalValueFxp = 0;
    int32_t currValueFxp = 0;
    bool waving = false;
    bool visible = false;
    uint64_t animationStartMs = 0;
    int32_t animationParam = 0;

    const char *message = nullptr;

    ProgressBar(const TinyFont &font, int w, int h = -1) :
        font(font),
        width(w),
        height(h >= 0 ? h : (font.height * 2 + 1 + PADDING * 2))
        { }

    void setValue(uint64_t nowMs, int32_t valueFxp) {
        goalValueFxp = valueFxp;
    }

    void setVisible(uint64_t nowMs, bool value) {
        if (value == this->visible) return;
        this->visible = value;
        animationStartMs = nowMs;
        animationParam = 0;
    }

    void setWaving(bool value) {
        if (value == this->waving) return;
        this->waving = value;
    }

    void setMessage(const char* value) {
        if (value == this->message) return;
        this->message = value;
    }

    void update(uint64_t nowMs) {
        currValueFxp = fxp12::follow(currValueFxp, goalValueFxp, fxp12::ONE / 8);
        if (nowMs < animationStartMs + ANIMATION_DURATION_MS) {
            animationParam = (nowMs - animationStartMs) * fxp12::ONE / ANIMATION_DURATION_MS;
        }
        else {
            animationParam = fxp12::ONE;
        }
        if (!visible) {
            animationParam = fxp12::ONE - animationParam;
        }
    }

    void render(uint64_t nowMs, ssd1306::Screen &g, int x0, int y0) {
        Rect clip(0, 0, width, height);
        if (animationParam <= 0) {
            return;
        }
        else if (animationParam < fxp12::ONE / 2) {
            int32_t p = animationParam * 2;
            if (visible) {
                p = fxp12::ONE - p;
                p = p * p / fxp12::ONE;
            }
            else {
                p = p * p / fxp12::ONE;
                p = fxp12::ONE - p;
            }
            int x = fxp12::roundToInt((width - 1) / 2 * p);
            clip.x = x;
            clip.y = (height - 1) / 2;
            clip.w = width - x * 2;
            clip.h = 1;
        }
        else if (animationParam < fxp12::ONE) {
            int32_t p = animationParam * 2 - fxp12::ONE;
            if (visible) {
                p = fxp12::ONE - p;
                p = p * p / fxp12::ONE;
            }
            else {
                p = p * p / fxp12::ONE;
                p = fxp12::ONE - p;
            }
            int y = fxp12::roundToInt((height / 2 - 1) * p);
            clip.x = 0;
            clip.y = y;
            clip.w = width;
            clip.h = height - y * 2;
        }
        clip.offsetSelf(x0, y0);

        g.setClipRect(clip);

        g.fillRect(clip, pen_t::BLACK);
        g.drawRect(clip.x, clip.y, clip.w - 1, clip.h - 1);

        if (message) g.drawString(font, x0 + PADDING, y0 + PADDING, message);

        Rect bar;
        bar.w = width - PADDING * 2,
        bar.h = height - font.height - PADDING * 2 - 1,
        bar.x = x0 + PADDING,
        bar.y = y0 + height - PADDING - bar.h;
        g.drawRect(bar.x, bar.y, bar.w - 1, bar.h - 1);

        bar.inflateSelf(-1, -1);
        if (waving) {
            bar.x += SHPC_ROUND_DIV(shapoco::abs((int)(nowMs % 1024) - 512) * (bar.w * 3 / 4), 512);
            bar.w /= 4;
        }
        else {
            bar.w = fxp12::roundToInt(bar.w * currValueFxp);
        }
        g.fillRect(bar);
        g.clearClipRect();
    }
};

}
