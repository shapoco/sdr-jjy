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

    Screen screen;

protected:
    int currPage = 0;
    int numSentPages = 0;

public:
    DriverBase() : screen(W, H) { }

    virtual void init() {
        currPage = 0;
        numSentPages = screen.numPages;

        screen.clear();

        setDisp(false);
        writeCmd(cmd_t::SET_MEM_MODE, 0);
        writeCmd(cmd_t::SET_DISP_START_LINE);
        writeCmd(cmd_t::SET_SEG_REMAP | 0x01);
        writeCmd(cmd_t::SET_MUX_RATIO, H - 1);
        writeCmd(cmd_t::SET_COM_OUT_DIR | 0x08);
        writeCmd(cmd_t::SET_DISP_OFFSET, 0);
        writeCmd(cmd_t::SET_COM_PIN_CFG, 
            (W == 128 && H == 32) ? 0x02 :
            (W == 128 && H == 64) ? 0x12 : 0x02);
        writeCmd(cmd_t::SET_DISP_CLK_DIV, 0x80);
        writeCmd(cmd_t::SET_PRECHARGE, 0xF1);
        writeCmd(cmd_t::SET_VCOM_DESEL, 0x30);
        writeCmd(cmd_t::SET_CONTRAST, 0xFF);
        writeCmd(cmd_t::SET_ENTIRE_ON);
        writeCmd(cmd_t::SET_NORM_DISP);
        writeCmd(cmd_t::SET_CHARGE_PUMP, 0x14);
        writeCmd(cmd_t::SET_SCROLL);

        writeCmd(cmd_t::SET_COL_ADDR, 0, W - 1);
        writeCmd(cmd_t::SET_PAGE_ADDR, 0, H - 1);
        writeBlocking(dc_t::DATA, screen.data, screen.sizeBytes);

        setDisp(true);
    }

    void setDisp(bool on) {
        writeCmd(cmd_t::SET_DISP | (on ? 0x01 : 0x00));
    }

    void commit(const Screen &back_buff) {
        memcpy(screen.data, back_buff.data, back_buff.sizeBytes);
        numSentPages = 0;
    }

    void service() {
        const int numPages = screen.numPages;
        if (numSentPages < numPages) {
            writeCmd(cmd_t::SET_COL_ADDR, 0, W - 1);
            writeCmd(cmd_t::SET_PAGE_ADDR, currPage, currPage);
            //writeDmaStart(dc_t::DATA, front_buff.back_buff + W * curr_page, sizeof(seg_t) * W);
            writeBlocking(dc_t::DATA, screen.data + W * currPage, sizeof(seg_t) * W);
            numSentPages += 1;
            if (numSentPages < numPages) {
                currPage = (currPage + 1) % numPages;
            }
            else {
                currPage = 0;
            }
        }
    }

    void writeCmd(const uint8_t cmd) {
        const uint8_t buf[] = { cmd };
        writeBlocking(dc_t::CMD, buf, 1);
    }

    void writeCmd(const uint8_t cmd, const uint8_t param0) {
        const uint8_t buf[] = { cmd, param0 };
        writeBlocking(dc_t::CMD, buf, 2);
    }

    void writeCmd(const uint8_t cmd, const uint8_t param0, const uint8_t param1) {
        const uint8_t buf[] = { cmd, param0, param1 };
        writeBlocking(dc_t::CMD, buf, 3);
    }

    virtual void writeBlocking(const dc_t dc, const void *src, const size_t size_in_bytes) = 0;
    virtual void writeDmaStart(const dc_t dc, const void *src, const size_t size_in_bytes) = 0;
    virtual void writeDmaComplete() = 0;
    virtual bool isDmaBusy() = 0;
};

}
