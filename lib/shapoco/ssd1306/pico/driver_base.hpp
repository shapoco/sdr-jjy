#pragma once

#include <stdint.h>
#include <string.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "shapoco/fixed12.hpp"
#include "shapoco/graphics/graphics.hpp"
#include "shapoco/ssd1306/ssd1306.hpp"

namespace shapoco::ssd1306::pico {

template<int W, int H>
class DriverBase {
public:
    static_assert(
        (W == 128 && H == 64) ||
        (W == 128 && H == 32));

    static constexpr int NUM_PAGES = (H + PAGE_HEIGHT - 1) / PAGE_HEIGHT;
    static constexpr int SCREEN_SIZE_BYTES = W * NUM_PAGES;

    Screen frontBuff;

protected:
    int currPage = 0;
    int numSentPages = 0;

    struct SegRange {
        int from;
        int to;
        void clear() { from = to = 0; }
        int size() const { return to - from; }
        int empty() const { return to == from; }
    };
    SegRange dirtyRanges[NUM_PAGES];

public:
    DriverBase() : frontBuff(W, H) { }

    virtual void init() {
        currPage = 0;
        numSentPages = NUM_PAGES;

        frontBuff.clear();

        for (int page = 0; page < NUM_PAGES; page++) {
            dirtyRanges[page].clear();
        }
        
        setDisp(false);

        writeCommand(cmd_t::SET_MUX_RATIO, H - 1);
        writeCommand(cmd_t::SET_DISP_OFFSET, 0);
        writeCommand(cmd_t::SET_DISP_START_LINE);
        writeCommand(cmd_t::SET_SEG_REMAP | 0x01);
        writeCommand(cmd_t::SET_COM_OUT_DIR | 0x08);
        writeCommand(cmd_t::SET_COM_PIN_CFG, 
            (W == 128 && H == 32) ? 0x02 :
            (W == 128 && H == 64) ? 0x12 : 0x02);
        writeCommand(cmd_t::SET_CONTRAST, 0xFF);
        writeCommand(cmd_t::SET_ENTIRE_ON);
        writeCommand(cmd_t::SET_NORM_DISP);
        writeCommand(cmd_t::SET_DISP_CLK_DIV, 0x80);
        writeCommand(cmd_t::SET_CHARGE_PUMP, 0x14);
        
        for (int page = 0; page < NUM_PAGES; page++) {
            writeCommand(cmd_t::SET_COL_ADDR, 0, W - 1);
            writeCommand(cmd_t::SET_PAGE_ADDR, page, page);
            writeDataBlocking(frontBuff.pagePtr(page), sizeof(seg_t) * W);
        }

        setDisp(true);
    }

    void setDisp(bool on) {
        writeCommand(cmd_t::SET_DISP | (on ? 0x01 : 0x00));
    }

    void commit(const Screen &backBuff) {
        for (int page = 0; page < NUM_PAGES; page++) {
            SegRange dirty;
            dirty.from = 0;
            dirty.to = W;

            {
                const seg_t *srcPtr = backBuff.pagePtr(page);
                const seg_t *dstPtr = frontBuff.pagePtr(page);
                while(dirty.from < W) {
                    if (*(srcPtr++) != *(dstPtr++)) break;
                    dirty.from++;
                }
            }
            
            {
                const seg_t *srcPtr = backBuff.pagePtr(page) + W;
                const seg_t *dstPtr = frontBuff.pagePtr(page) + W;
                while (dirty.from < dirty.to) {
                    if (*(--srcPtr) != *(--dstPtr)) break;
                    dirty.to--;
                }
            }

            if (!dirty.empty()) {
                const seg_t *srcPtr = backBuff.pagePtr(page) + dirty.from;
                seg_t *dstPtr = frontBuff.pagePtr(page) + dirty.from;
                memcpy(dstPtr, srcPtr, sizeof(seg_t) * dirty.size());
            }

            if (dirtyRanges[page].empty()) {
                dirtyRanges[page] = dirty;
            }
            else {
                dirtyRanges[page].from = SHPC_MIN(dirtyRanges[page].from, dirty.from);
                dirtyRanges[page].to = SHPC_MAX(dirtyRanges[page].to, dirty.to);
            }
        }

        numSentPages = 0;
    }

    void service() {
        while (numSentPages < NUM_PAGES) {
            SegRange &dirty = dirtyRanges[currPage];
            bool empty = dirty.empty();
            if (!empty) {
                writeCommand(cmd_t::SET_COL_ADDR, dirty.from, dirty.to - 1);
                writeCommand(cmd_t::SET_PAGE_ADDR, currPage, currPage);
                writeDataAsync(frontBuff.pagePtr(currPage) + dirty.from, sizeof(seg_t) * dirty.size());
                dirty.clear();
            }
            
            numSentPages += 1;
            if (numSentPages < NUM_PAGES) {
                currPage = (currPage + 1) % NUM_PAGES;
            }
            else {
                currPage = 0;
            }

            if (!empty) break;
        }
    }

    void writeCommand(uint8_t cmd) {
        const uint8_t buf[] = { cmd };
        writeBlocking(dc_t::COMMAND, buf, sizeof(buf));
    }

    void writeCommand(uint8_t cmd, uint8_t param0) {
        const uint8_t buf[] = { cmd, param0 };
        writeBlocking(dc_t::COMMAND, buf, sizeof(buf));
    }

    void writeCommand(uint8_t cmd, uint8_t param0, uint8_t param1) {
        const uint8_t buf[] = { cmd, param0, param1 };
        writeBlocking(dc_t::COMMAND, buf, sizeof(buf));
    }

    void writeDataBlocking(const void *src, size_t size_in_bytes) {
        writeBlocking(dc_t::DATA, src, size_in_bytes);
    }
    
    virtual void writeDataAsync(const void *src, size_t size_in_bytes) = 0;
    virtual void writeDataFlush() = 0;

    virtual void writeBlocking(dc_t dc, const void *src, size_t size_in_bytes) = 0;
    virtual bool isDmaBusy() = 0;
};

}
