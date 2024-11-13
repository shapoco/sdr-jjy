#pragma once

#include <stdint.h>

namespace shapoco::ssd1306 {

static constexpr int PAGE_HEIGHT = 8;

typedef enum : bool {
    CMD = false,
    DATA = true,
} dc_t;

typedef enum {
    SET_MEM_MODE        = 0x20,
    SET_COL_ADDR        = 0x21,
    SET_PAGE_ADDR       = 0x22,
    SET_HORIZ_SCROLL    = 0x26,
    SET_SCROLL          = 0x2E,
    SET_DISP_START_LINE = 0x40,
    SET_CONTRAST        = 0x81,
    SET_CHARGE_PUMP     = 0x8D,
    SET_SEG_REMAP       = 0xA0,
    SET_ENTIRE_ON       = 0xA4,
    SET_ALL_ON          = 0xA5,
    SET_NORM_DISP       = 0xA6,
    SET_INV_DISP        = 0xA7,
    SET_MUX_RATIO       = 0xA8,
    SET_DISP            = 0xAE,
    SET_COM_OUT_DIR     = 0xC0,
    SET_COM_OUT_DIR_FLIP= 0xC0,
    SET_DISP_OFFSET     = 0xD3,
    SET_DISP_CLK_DIV    = 0xD5,
    SET_PRECHARGE       = 0xD9,
    SET_COM_PIN_CFG     = 0xDA,
    SET_VCOM_DESEL      = 0xDB,
} cmd_t;

using seg_t = uint8_t;

typedef enum : uint8_t {
    BLACK = 0,
    WHITE = 1,
} pen_t;

}
