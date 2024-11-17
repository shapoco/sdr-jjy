#pragma once

#include <stdio.h>
#include <string.h>

#include "shapoco/jjy/jjy.hpp"
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

class BufferView {
public:
    static constexpr int TITLE_HEIGHT = 6;
    static constexpr int CELL_W = 4;
    static constexpr int CELL_H = 5;
    static constexpr int NUM_COLS = 11;

    static constexpr int WIDTH = CELL_W * (NUM_COLS- 1) + 6;
    static constexpr int HEIGHT = 44;

    static constexpr int NUM_ROWS = SHPC_CEIL_DIV(HEIGHT - TITLE_HEIGHT, CELL_H);

    static constexpr int ANIMATION_DURATION_MS = 200;

    RotaryCounter indexNumber;

    struct cell_t {
        bool valid;
        jjy::jjybit_t value;

        void clear(uint64_t t_ms) {
            valid = false;
            value = jjy::jjybit_t::ERROR;
        }
    };

    struct Row {
        cell_t cells[NUM_COLS];
        bool hasSeparator;
        int goalX = 0, goalY = 0;
        int offsetX = 0, offsetY = 0;
        uint64_t tAnimationEndMs;

        void clear(uint64_t t_ms, int irow) {
            for (int i = 0; i < NUM_COLS; i++) {
                cells[i].clear(t_ms);
            }
            moveTo(t_ms, 0, irow * CELL_H, false);
            hasSeparator = false;
        }

        void moveTo(uint64_t t_ms, int newX, int newY, bool animation) {
            if (animation) {
                int currX, currY;
                getCurrentPos(t_ms, &currX, &currY);
                goalX = newX;
                goalY = newY;
                offsetX = currX - newX;
                offsetY = currY - newY;
                tAnimationEndMs = t_ms + ANIMATION_DURATION_MS;
            }
            else {
                goalX = newX;
                goalY = newY;
                offsetX = 0;
                offsetY = 0;
                tAnimationEndMs = t_ms;
            }
        }

        void getCurrentPos(uint64_t t_ms, int *x, int *y) const {
            if (t_ms < tAnimationEndMs) {
                int d = tAnimationEndMs - t_ms;
                if (x) *x = goalX + SHPC_ROUND_DIV(offsetX * d, ANIMATION_DURATION_MS);
                if (y) *y = goalY + SHPC_ROUND_DIV(offsetY * d, ANIMATION_DURATION_MS);
            }
            else {
                if (x) *x = goalX;
                if (y) *y = goalY;
            }
        }
    };
    

private:
    Row rows[NUM_ROWS];
    bool lastToggle = true;

public:
    BufferView() : indexNumber(fonts::font5, 0, 59) {}

    void init(uint64_t nowMs) {
        for (int irow = 0; irow < NUM_ROWS; irow++) {
            rows[irow].clear(nowMs, irow);
        }
        layoutRows(nowMs, true);
    }

    void render(uint64_t nowMs, ssd1306::Screen &g, int x0, int y0, const receiver_status_t &sts) {
        indexNumber.update(nowMs);

        if (sts.dec.toggle != lastToggle) {
            lastToggle = sts.dec.toggle;
            updateTable(nowMs, sts);
        }
        
        g.drawString(fonts::font5, x0, y0, "BUFF");
        if (sts.dec.synced) {
            indexNumber.render(g, x0 + WIDTH - indexNumber.width, y0);
        }

        bool blink = jjy::phaseAdd(sts.sync.phase_cursor, -sts.sync.phase_offset) < jjy::PHASE_PERIOD / 2;

        g.setClipRect(x0, y0 + TITLE_HEIGHT, (NUM_COLS - 1) * CELL_W, HEIGHT - TITLE_HEIGHT);
        for (int irow = NUM_ROWS - 1; irow >= 0; irow--) {
            Row &row = rows[irow];
            int rowX, rowY;
            row.getCurrentPos(nowMs, &rowX, &rowY);
            rowX += x0;
            rowY += y0;
            int cellX = rowX + (NUM_COLS - 1) * CELL_W;
            for (int icol = 0; icol < NUM_COLS; icol++) {
                cellX -= CELL_W;
                const cell_t &cell = row.cells[icol];
                if (!cell.valid) {
                    g.drawChar(fonts::font4, cellX, rowY, '.');
                }
                else if (cell.value == jjy::jjybit_t::ZERO) {
                    g.drawChar(fonts::font4, cellX, rowY, '0');
                }
                else if (cell.value == jjy::jjybit_t::ONE) {
                    g.drawChar(fonts::font4, cellX, rowY, '1');
                }
                else if (cell.value == jjy::jjybit_t::MARKER) {
                    g.drawChar(fonts::font4, cellX, rowY, 'M');
                }
                else {
                    g.drawChar(fonts::font4, cellX, rowY, blink ? 'X' : 'x');
                }
            }

            if (row.hasSeparator) {
                g.fillRect(x0, rowY + CELL_H, (NUM_COLS - 1) * CELL_W - 1, 1);
            }
        }
        g.clearClipRect();

#if 0
        {
            int y = y0 + HEIGHT + 2;

            sprintf(s, "synced=%d", sts.dec.synced ? 1 : 0);
            lcd.drawString(bmpfont::font5, x0, y, s);
            y += 6;

            char c =
                sts.dec.last_bit_value == jjy::jjybit_t::ZERO ? '0' :
                sts.dec.last_bit_value == jjy::jjybit_t::ONE ? '1' :
                sts.dec.last_bit_value == jjy::jjybit_t::MARKER ? 'M' :
                sts.dec.last_bit_value == jjy::jjybit_t::ERROR ? 'E' : '?';
            sprintf(s, "last=%d,%c", sts.dec.last_bit_index, c);
            lcd.drawString(bmpfont::font5, x0, y, s);
            y += 6;
        }
#endif
    }

    void updateTable(uint64_t nowMs, const receiver_status_t &sts) {
        bool feed = 
            (sts.dec.last_action == jjy::rx::Decoder::action_t::SYNC_MARKER) ||
            (sts.dec.last_action == jjy::rx::Decoder::action_t::TICK_CONTINUE && sts.dec.last_bit_index % 10 == 0);
        if (feed) {
            bool add_sep = sts.dec.synced && sts.dec.last_bit_index == 0;
            feedLine(nowMs, add_sep);
        }
        shiftIn(nowMs, sts.dec.last_bit_value);
        indexNumber.setNumber(nowMs, sts.dec.last_bit_index);
    }

    void shiftIn(uint64_t nowMs, jjy::jjybit_t in) {
        Row &row = rows[NUM_ROWS - 1];
        for (int icol = NUM_COLS - 1; icol >= 1; icol--) {
            row.cells[icol] = row.cells[icol - 1];
        }
        cell_t &cell = row.cells[0];
        cell.clear(nowMs);
        cell.value = in;
        cell.valid = true;

        int y;
        row.getCurrentPos(nowMs, nullptr, &y);
        row.moveTo(nowMs, CELL_W - 1, y, false);
        row.moveTo(nowMs, 0, y, true);
    }

    void feedLine(uint64_t nowMs, bool addSeparator) {
        for (int irow = 0; irow < NUM_ROWS - 1; irow++) {
            rows[irow] = rows[irow + 1];
        }
        rows[NUM_ROWS - 2].hasSeparator = addSeparator;
        rows[NUM_ROWS - 1].clear(nowMs, NUM_ROWS - 1);
        layoutRows(nowMs, false);
    }

    void layoutRows(uint64_t nowMs, bool reset) {
        int y = HEIGHT - CELL_H + 1;
        for (int irow = NUM_ROWS - 1; irow >= 0; irow--) {
            Row &row = rows[irow];
            if (irow < NUM_ROWS - 1 && row.hasSeparator) {
                y -= 2;
            }
            row.moveTo(nowMs, 0, y, !reset && irow != NUM_ROWS - 1);
            y -= CELL_H;
        }
    }
    
};

}
