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

    struct Digit {
        int x;
        int currNumber;
        int nextNumber;
        bool currVisible;
        bool nextVisible;
        int shift;
    };

    const TinyFont &font;
    const int minValue;
    const int maxValue;
    const int numDigits;
    const int numVisibleDigits;
    const int dotPos;
    const int width;
    const int height;

    bool zeroPadding = false;
    bool forwardOnly = false;
    int32_t followRatio = fxp12::ONE / 2;
    int32_t followStepMax = fxp12::ONE / 4;

    int32_t goalValueFxp = 0;
    int32_t currValueFxp = 0;

    Digit * const digits;

public:
    RotaryCounter(const TinyFont &font, int minValue, int maxValue, int numVisibleDigits = -1, int dotPos = 0) : 
        font(font), 
        minValue(minValue),
        maxValue(maxValue),
        numDigits(shapoco::clog10(1 + SHPC_MAX(SHPC_ABS(minValue), SHPC_ABS(maxValue)))), 
        numVisibleDigits(numVisibleDigits > 0 ? numVisibleDigits : numDigits),
        dotPos(dotPos),
        width(
            this->numVisibleDigits * (font.get_char_info('0').width + font.spacing) - font.spacing +
            ((dotPos > 0) ? (font.get_char_info('.').width + font.spacing) : 0)
        ), 
        height(font.height),
        digits(new Digit[numDigits])
    { 
        int digitWidth = font.get_char_info('0').width;
        int x = width - digitWidth;
        int max = SHPC_MAX(SHPC_ABS(minValue), SHPC_ABS(maxValue));
        for (int idig = 0; idig < numDigits; idig++) {
            Digit &dig = digits[idig];
            dig.x = x;
            dig.currNumber = 0;
            dig.nextNumber = 1;
            dig.currVisible = zeroPadding || (idig == 0);
            dig.nextVisible = zeroPadding || (idig == 0);
            x -= digitWidth + font.spacing;
            max /= 10;
        }
    }

    ~RotaryCounter() {
        delete[] digits;
    }

    void setNumber(int value) {
        goalValueFxp = value * fxp12::ONE;
    }

    void update(uint64_t t_ms) {
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
        int currValue = currValueFxp / fxp12::ONE;
        int nextValue = currValue + 1 < period ? currValue + 1 : 0;
        currValueFxp += minValueFxp;
        goalValueFxp += minValueFxp;
        currValue += minValue;
        nextValue += minValue;

        currValue = SHPC_ABS(currValue);
        nextValue = SHPC_ABS(nextValue);
        int32_t shiftFxp = currValueFxp & (fxp12::ONE - 1);
        const int yStride = (font.height + 2);
        for (int idig = 0; idig < numDigits; idig++) {
            Digit &dig = digits[idig];
            dig.currNumber = currValue % 10;
            dig.nextNumber = nextValue % 10;
            dig.shift = SHPC_ROUND_DIV(shiftFxp * yStride, fxp12::ONE);
            if (dig.currNumber != 9) shiftFxp = 0;
            currValue /= 10;
            nextValue /= 10;
        }

        bool currVisible = zeroPadding;
        bool nextVisible = zeroPadding;
        for (int idig = numDigits - 1; idig >= 0; idig--) {
            Digit &dig = digits[idig];
            currVisible |= (dig.currNumber != 0);
            nextVisible |= (dig.nextNumber != 0);
            dig.currVisible = currVisible || (idig == 0);
            dig.nextVisible = nextVisible || (idig == 0);
        }
    }

    void render(ssd1306::Screen &g, int x0, int y0) {
        g.setClipRect(x0, y0 - 1, width, height + 2);
        const int yStride = (font.height + 2);
        for (int idig = 0; idig < numDigits; idig++) {
            Digit &dig = digits[idig];
            if (dig.currNumber != 0 || dig.currVisible) {
                g.drawChar(font, x0 + dig.x, y0 + dig.shift, '0' + dig.currNumber);
            }
            if (dig.shift != 0 && (dig.nextNumber != 0 || dig.nextVisible)) {
                g.drawChar(font, x0 + dig.x, y0 + dig.shift - yStride, '0' + dig.nextNumber);
            }
        }
        g.clearClipRect();
    }

};

}
