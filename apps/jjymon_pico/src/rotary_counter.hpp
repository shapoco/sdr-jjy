#pragma once

#include "shapoco/math_utils.hpp"
#include "shapoco/fixed12.hpp"
#include "shapoco/graphics/graphics.hpp"
#include "shapoco/ssd1306/ssd1306.hpp"

#include "ui.hpp"
#include "fonts.hpp"

namespace shapoco::jjymon {

using namespace ::shapoco::graphics;
using pen_t = ::shapoco::ssd1306::pen_t;

class RotaryCounter {
public:
    static constexpr int32_t POS_PERIOD = 10 * fxp12::ONE;
    static constexpr int ANIMATION_DURATION_MS = 250;

    enum BlankState {
        NORMAL,
        BLANK,
        BLANK_EXITING,
    };

    struct Digit {
        int number;
        bool zeroVisible;
        bool blank;
    };

    struct Place {
        int x;
        Digit currDig;
        Digit nextDig;
        int yShift;
    };

    const TinyFont &font;
    const int minValue;
    const int maxValue;
    const int numPlaces;
    const int numVisiblePlaces;
    const int dotPos;
    int width;
    const int height;
    const int yStride;
    Place * const places;

    bool zeroPadding = false;
    bool forwardOnly = false;
    int32_t followRatio = fxp12::ONE / 2;
    int32_t followStepMax = fxp12::ONE / 4;

    int32_t goalValueFxp = 0;
    int32_t currValueFxp = 0;
    BlankState blankState = BlankState::NORMAL;
    uint64_t tAnimationStartMs = 0;

    const char **aliases = nullptr;

    RotaryCounter(const TinyFont &font, int minValue, int maxValue, int numVisiblePlaces = -1, int dotPos = 0) : 
        font(font), 
        minValue(minValue),
        maxValue(maxValue),
        numPlaces(shapoco::clog10(1 + SHPC_MAX(shapoco::abs(minValue), shapoco::abs(maxValue)))), 
        numVisiblePlaces(numVisiblePlaces > 0 ? numVisiblePlaces : numPlaces),
        dotPos(dotPos),
        width(
            this->numVisiblePlaces * (font.getGlyph('0')->width + font.spacing) - font.spacing +
            ((dotPos > 0) ? (font.getGlyph('.')->width + font.spacing) : 0)
        ), 
        height(font.height),
        yStride(font.height + 2),
        places(new Place[numPlaces])
    { 
        int digitWidth = font.getGlyph('0')->width;
        int x = width - digitWidth;
        int max = SHPC_MAX(shapoco::abs(minValue), shapoco::abs(maxValue));
        for (int idig = 0; idig < numPlaces; idig++) {
            Place &place = places[idig];
            place.x = x;
            place.currDig.number = 0;
            place.nextDig.number = 1;
            place.currDig.zeroVisible = zeroPadding || (idig == 0);
            place.nextDig.zeroVisible = zeroPadding || (idig == 0);
            x -= digitWidth + font.spacing;
            max /= 10;
        }
    }

    ~RotaryCounter() {
        delete[] places;
    }

    void setAliases(const char **aliases, bool adjustWidth = true) {
        this->aliases = aliases;
        if (adjustWidth) {
            int dw = 0;
            for (int i = minValue; i <= maxValue; i++) {
                dw = SHPC_MAX(dw, font.measureStringWidth(aliases[i]));
            }
            width = 50; //(dw + font.spacing) * numDigits - font.spacing;
        }
    }

    void setNumber(uint64_t nowMs, int value, bool animation = true) {
        goalValueFxp = value * fxp12::ONE;
        if (animation) {
            int periodFxp = fxp12::ONE * (maxValue + 1 - minValue);
            if (blankState == BlankState::BLANK) {
                // ブランク状態から解放
                blankState = BlankState::BLANK_EXITING;
                tAnimationStartMs = nowMs;
                if (forwardOnly || 0 <= shapoco::cyclicDiff(goalValueFxp, currValueFxp, periodFxp)) {
                    // 正転
                    currValueFxp = fxp12::floor(currValueFxp);
                }
                else {
                    // 逆転
                    currValueFxp = fxp12::floor(shapoco::cyclicAdd(currValueFxp, -fxp12::ONE, periodFxp));
                }
            }
        }
        else {
            currValueFxp = goalValueFxp;
            blankState = BlankState::NORMAL;
        }
    }

    void setBlank() {
        currValueFxp = goalValueFxp = 0;
        tAnimationStartMs = 0;
        blankState = BlankState::BLANK;
    }

    void update(uint64_t nowMs) {
        if (blankState == BlankState::NORMAL) {
            updateForNormal(nowMs);
        }
        else {
            updateForBlank(nowMs);
        }

        // ゼロサプレス
        bool currZeroVisible = zeroPadding;
        bool nextZeroVisible = zeroPadding;
        for (int idig = numPlaces - 1; idig >= 0; idig--) {
            Place &place = places[idig];
            currZeroVisible |= (place.currDig.number != 0);
            nextZeroVisible |= (place.nextDig.number != 0);
            place.currDig.zeroVisible = currZeroVisible || (idig == 0);
            place.nextDig.zeroVisible = nextZeroVisible || (idig == 0);
        }
    }

    void updateForNormal(uint64_t nowMs) {
        // 表示値の更新
        int period = maxValue + 1 - minValue;
        int32_t periodFxp = period * fxp12::ONE;
        int32_t minValueFxp = minValue * fxp12::ONE;
        currValueFxp -= minValueFxp;
        goalValueFxp -= minValueFxp;
        if (forwardOnly) {
            int32_t diff = goalValueFxp - currValueFxp;
            if (diff < 0) diff += periodFxp;
            if (diff != 0) {
                int32_t step = FXP_CLIP(1, followStepMax, diff * followRatio / fxp12::ONE);
                currValueFxp = shapoco::cyclicAdd(currValueFxp, step, periodFxp);
            }
        }
        else {
            currValueFxp = shapoco::cyclicFollow(currValueFxp, goalValueFxp, periodFxp, followRatio, fxp12::PREC, followStepMax);
        }
        int currValue = fxp12::truncToInt(currValueFxp);
        int nextValue = shapoco::cyclicAdd(currValue, 1, period);
        currValueFxp += minValueFxp;
        goalValueFxp += minValueFxp;
        currValue += minValue;
        nextValue += minValue;

        // 表示盤の数字と位置の更新
        currValue = shapoco::abs(currValue);
        nextValue = shapoco::abs(nextValue);
        int32_t yShiftFxp = currValueFxp & (fxp12::ONE - 1);
        for (int idig = 0; idig < numPlaces; idig++) {
            Place &place = places[idig];
            place.currDig.number = currValue % 10;
            place.nextDig.number = nextValue % 10;
            place.currDig.blank = false;
            place.nextDig.blank = false;
            place.yShift = fxp12::roundToInt(yShiftFxp * yStride);
            if (place.currDig.number != 9) yShiftFxp = 0;
            currValue /= 10;
            nextValue /= 10;
        }
    }

    void updateForBlank(uint64_t nowMs) {
        // 表示盤の数字と位置の更新
        int nextValue = fxp12::truncToInt(currValueFxp);
        int yShift = 0;
        if (blankState == BlankState::BLANK_EXITING) {
            if (nowMs < tAnimationStartMs + ANIMATION_DURATION_MS) {
                int32_t yShiftFxp = (nowMs - tAnimationStartMs) * fxp12::ONE / ANIMATION_DURATION_MS;
                yShift = fxp12::roundToInt(yShiftFxp * yStride);
            }
            else {
                yShift = yStride;
                blankState = BlankState::NORMAL;
            }
        }
        for (int idig = 0; idig < numPlaces; idig++) {
            Place &place = places[idig];
            place.currDig.number = nextValue % 10;
            place.nextDig.number = nextValue % 10;
            place.currDig.blank = true;
            place.nextDig.blank = false;
            place.yShift = yShift;
            nextValue /= 10;
        }
    }

    void render(ssd1306::Screen &g, int x0, int y0) {
        g.setClipRect(x0, y0 - 1, width, height + 2);
        for (int idig = 0; idig < numPlaces; idig++) {
            Place &place = places[idig];
            renderDigit(g, x0 + place.x, y0 + place.yShift, place.currDig);
            if (place.yShift != 0) {
                renderDigit(g, x0 + place.x, y0 + place.yShift - yStride, place.nextDig);
            }
        }
        g.clearClipRect();
    }

    void renderDigit(ssd1306::Screen &g, int x, int y, Digit &dig) {
        if (aliases) {
            g.drawString(font, x, y, aliases[dig.number]);
        }
        else if (dig.blank) {
            g.drawChar(font, x, y, '-');
        }
        else if (dig.number != 0 || dig.zeroVisible) {
            g.drawChar(font, x, y, '0' + dig.number);
        }
    }

};

}
