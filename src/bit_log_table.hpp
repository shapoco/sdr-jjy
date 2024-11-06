#ifndef BIT_LOG_TABLE_HPP
#define BIT_LOG_TABLE_HPP

#include <stdio.h>
#include <string.h>

#include "jjymon.hpp"
#include "jjy/jjy.hpp"
#include "fixed12.hpp"
#include "ssd1309spi.hpp"
#include "images/images.hpp"
#include "bmpfont/bmpfont.hpp"

using pen_t = ssd1309spi::pen_t;

class BitLogTable {
public:
    static constexpr int COLS = 11;
    static constexpr int ROWS = 7;
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

private:
    cell_t cells[COLS * ROWS];
    bool last_toggle = true;
    int x_shift = 0;
    int y_shift = 0;

public:
    void init(uint64_t t_now_ms) {
        for (int i = 0; i < COLS * ROWS; i++) {
            cells[i].valid = false;
        }
        x_shift = 0;
        y_shift = 0;
    }

    void render(uint64_t t_now_ms, JjyLcd &lcd, int x0, int y0, const receiver_status_t &sts) {
        char s[16];

        if (sts.dec.toggle != last_toggle) {
            last_toggle = sts.dec.toggle;
            update_table(t_now_ms, sts);
        }

        bool blink = jjy::phase_add(sts.sync.phase_cursor, -sts.sync.phase_offset) < jjy::PHASE_PERIOD / 2;

        for (int row = 0; row < ROWS; row++) {
            int y = y0 + 6 + (row - 1) * CELL_H + y_shift;
            int x = x0 + COLS * CELL_W;
            if (row == ROWS - 1) x -= x_shift;
            for (int col = 0; col < COLS; col++) {
                x -= CELL_W;
                if (col == 0 && (row != ROWS - 1 || x_shift == 0)) continue;
                const cell_t &cell = cells[row * COLS + col];
                if (!cell.valid) continue;

                switch (cell.value) {
                case jjy::jjybit_t::ZERO:
                    lcd.draw_char(bmpfont::font4, x, y, '0');
                    break;

                case jjy::jjybit_t::ONE:
                    lcd.draw_char(bmpfont::font4, x, y, '1');
                    break;

                case jjy::jjybit_t::MARKER:
                    lcd.draw_char(bmpfont::font4, x, y, blink ? 'M' : 'N');
                    break;

                case jjy::jjybit_t::ERROR:
                default:
                    if (blink) lcd.draw_char(bmpfont::font4, x, y, 'E');
                    break;
                }
            }
        }

        x_shift = FXP_MAX(0, x_shift - 1);
        y_shift = FXP_MAX(0, y_shift - 1);

        lcd.fill_rect(x0 -CELL_W, y0,  CELL_W * (COLS + 2), 6, pen_t::BLACK);
        lcd.draw_string(bmpfont::font5, x0, y0, "DATA");
        if (sts.dec.synced) {
            sprintf(s, "%d", sts.dec.last_bit_index);
            lcd.draw_string(bmpfont::font5, x0 + 30, y0, s);
        }

        {
            int y = y0 + ROWS * CELL_H;

            sprintf(s, "synced=%d", sts.dec.synced ? 1 : 0);
            lcd.draw_string(bmpfont::font5, x0, y, s);
            y += 6;

            sprintf(s, "num_mkr=%d", sts.dec.num_marker_found);
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
    }

    void update_table(uint64_t t_now_ms, const receiver_status_t &sts) {
        bool feed = 
            (sts.dec.last_action == jjy::rx::Decoder::action_t::SYNC_FINISH) ||
            (sts.dec.last_action == jjy::rx::Decoder::action_t::TICK_CONTINUE && sts.dec.last_bit_index % 10 == 0);
        if (feed) {
            feed_line(t_now_ms);
        }

        shift_in(t_now_ms, sts.dec.last_bit_value);

    }

    void shift_in(uint64_t t_now_ms, jjy::jjybit_t in) {
        memcpy(&cells[(ROWS - 1) * COLS], &cells[(ROWS - 1) * COLS + 1], sizeof(cell_t) * (COLS - 1));
        cell_t &new_cell = cells[ROWS * COLS - 1];
        new_cell.clear(t_now_ms);
        new_cell.value = in;
        new_cell.valid = true;
        x_shift = CELL_W - 1;
    }

    void feed_line(uint64_t t_now_ms) {
        for (int row = 1; row < ROWS; row++) {
            memcpy(&cells[(row - 1) * COLS], &cells[row * COLS], sizeof(cell_t) * COLS);
        }
        for (int col = 0; col < COLS; col++) {
            cells[(ROWS - 1) * COLS + col].clear(t_now_ms);
        }
        y_shift = CELL_H - 1;
    }
    
};

#endif
