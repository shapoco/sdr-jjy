#ifndef BIT_LOG_TABLE_HPP
#define BIT_LOG_TABLE_HPP

#include <stdio.h>
#include <string.h>

#include "shapoco/jjy/jjy.hpp"
#include "shapoco/fixed12.hpp"
#include "shapoco/graphics/graphics.hpp"
#include "shapoco/pico/ssd1309spi.hpp"

#include "jjymon.hpp"
#include "ui.hpp"
#include "images.hpp"
#include "fonts.hpp"

namespace shapoco::jjymon {

using namespace ::shapoco::graphics;
using pen_t = ::shapoco::pico::pen_t;

class BufferView {
public:
    static constexpr int WIDTH = 50;
    static constexpr int HEIGHT = 40;

    static constexpr int TITLE_H = 6;
    static constexpr int NUM_COLS = 11;
    static constexpr int NUM_ROWS = 7;
    static constexpr int CELL_W = 4;
    static constexpr int CELL_H = 5;

    struct cell_t {
        bool valid;
        jjy::jjybit_t value;

        void clear(uint64_t t_now_ms) {
            valid = false;
            value = jjy::jjybit_t::ERROR;
        }
    };

    struct Row {
        cell_t cells[NUM_COLS];
        int goal_x = 0, goal_y = 0;
        int disp_x = 0, disp_y = 0;
        bool has_separator;

        void clear(uint64_t t_ms, int irow) {
            for (int i = 0; i < NUM_COLS; i++) {
                cells[i].clear(t_ms);
            }
            goal_x = disp_x = 0;
            goal_y = disp_y = irow * CELL_H;
            has_separator = false;
        }

        void animate(uint64_t t_ms) {
            if (disp_x < goal_x) disp_x++;
            else if (disp_x > goal_x) disp_x--;
            if (disp_y < goal_y) disp_y++;
            else if (disp_y > goal_y) disp_y--;
        }
    };
    

private:
    Row rows[NUM_ROWS];
    bool last_toggle = true;

public:
    void init(uint64_t t_now_ms) {
        for (int irow = 0; irow < NUM_ROWS; irow++) {
            rows[irow].clear(t_now_ms, irow);
        }
        layout_rows(true);
    }

    void render(uint64_t t_now_ms, JjyLcd &lcd, int x0, int y0, const receiver_status_t &sts) {
        char s[16];

        if (sts.dec.toggle != last_toggle) {
            last_toggle = sts.dec.toggle;
            update_table(t_now_ms, sts);
        }

        bool blink = jjy::phase_add(sts.sync.phase_cursor, -sts.sync.phase_offset) < jjy::PHASE_PERIOD / 2;

        for (int irow = NUM_ROWS - 1; irow >= 0; irow--) {
            Row &row = rows[irow];
            row.animate(t_now_ms);
            int cell_x = x0 + row.disp_x + (NUM_COLS - 1) * CELL_W;
            int row_y = y0 + row.disp_y;
            for (int icol = 0; icol < NUM_COLS; icol++) {
                cell_x -= CELL_W;
                const cell_t &cell = row.cells[icol];
                if (!cell.valid) {
                    lcd.draw_char(fonts::font4, cell_x, row_y, '.');
                }
                else if (cell.value == jjy::jjybit_t::ZERO) {
                    lcd.draw_char(fonts::font4, cell_x, row_y, '0');
                }
                else if (cell.value == jjy::jjybit_t::ONE) {
                    lcd.draw_char(fonts::font4, cell_x, row_y, '1');
                }
                else if (cell.value == jjy::jjybit_t::MARKER) {
                    lcd.draw_char(fonts::font4, cell_x, row_y, 'M');
                }
                else {
                    lcd.draw_char(fonts::font4, cell_x, row_y, blink ? 'X' : 'x');
                }
            }

            if (row.has_separator) {
                lcd.fill_rect(x0, row_y + CELL_H, (NUM_COLS - 1) * CELL_W - 1, 1);
            }
            
            lcd.fill_rect(x0 - CELL_W * 2, row_y,  CELL_W * 2, CELL_H, pen_t::BLACK);
            lcd.fill_rect(x0 + CELL_W * (NUM_COLS - 1), row_y, CELL_W * 2, CELL_H, pen_t::BLACK);
        }

        lcd.fill_rect(x0 - CELL_W, y0, WIDTH + CELL_W, TITLE_H, pen_t::BLACK);

        lcd.draw_string(fonts::font5, x0, y0, "BUFF");
        if (sts.dec.synced) {
            sprintf(s, "%d", sts.dec.last_bit_index);
            lcd.draw_string(fonts::font5, x0 + 30, y0, s);
        }

#if 0
        {
            int y = y0 + HEIGHT + 2;

            sprintf(s, "synced=%d", sts.dec.synced ? 1 : 0);
            lcd.draw_string(bmpfont::font5, x0, y, s);
            y += 6;

            char c =
                sts.dec.last_bit_value == jjy::jjybit_t::ZERO ? '0' :
                sts.dec.last_bit_value == jjy::jjybit_t::ONE ? '1' :
                sts.dec.last_bit_value == jjy::jjybit_t::MARKER ? 'M' :
                sts.dec.last_bit_value == jjy::jjybit_t::ERROR ? 'E' : '?';
            sprintf(s, "last=%d,%c", sts.dec.last_bit_index, c);
            lcd.draw_string(bmpfont::font5, x0, y, s);
            y += 6;
        }
#endif
    }

    void update_table(uint64_t t_now_ms, const receiver_status_t &sts) {
        bool feed = 
            (sts.dec.last_action == jjy::rx::Decoder::action_t::SYNC_MARKER) ||
            (sts.dec.last_action == jjy::rx::Decoder::action_t::TICK_CONTINUE && sts.dec.last_bit_index % 10 == 0);
        if (feed) {
            bool add_sep = sts.dec.synced && sts.dec.last_bit_index == 0;
            feed_line(t_now_ms, add_sep);
        }
        shift_in(t_now_ms, sts.dec.last_bit_value);
    }

    void shift_in(uint64_t t_now_ms, jjy::jjybit_t in) {
        Row &row = rows[NUM_ROWS - 1];
        for (int icol = NUM_COLS - 1; icol >= 1; icol--) {
            row.cells[icol] = row.cells[icol - 1];
        }
        cell_t &cell = row.cells[0];
        cell.clear(t_now_ms);
        cell.value = in;
        cell.valid = true;
        row.disp_x = CELL_W - 1;
    }

    void feed_line(uint64_t t_ms, bool add_separator) {
        for (int irow = 0; irow < NUM_ROWS - 1; irow++) {
            rows[irow] = rows[irow + 1];
        }
        rows[NUM_ROWS - 2].has_separator = add_separator;
        rows[NUM_ROWS - 1].clear(t_ms, NUM_ROWS - 1);
        layout_rows(false);
    }

    void layout_rows(bool reset) {
        int y = HEIGHT - CELL_H + 1;
        for (int irow = NUM_ROWS - 1; irow >= 0; irow--) {
            Row &row = rows[irow];
            if (irow < NUM_ROWS - 1 && row.has_separator) {
                y -= 2;
            }
            row.goal_y = y;
            if (reset || irow == NUM_ROWS - 1) {
                row.disp_y = y;
            }
            y -= CELL_H;
        }
    }
    
};

}

#endif
