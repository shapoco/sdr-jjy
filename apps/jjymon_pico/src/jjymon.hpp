#pragma once

#ifndef JJYMON_CFG_UI
#define JJYMON_CFG_UI ssd1309spi
#endif

#define ENABLE_STDOUT (1)

#include "hardware/clocks.h"

#include "shapoco/jjy/jjy.hpp"
#include "shapoco/pico/atomic.hpp"

namespace shapoco::jjymon {

static constexpr uint32_t SYS_CLK_FREQ = 150 * MHZ;

static constexpr uint32_t ADC_SPS = shapoco::jjy::rx::DETECTION_INPUT_SPS;
static constexpr uint32_t DMA_SIZE = shapoco::jjy::rx::DETECTION_BLOCK_SIZE;

static constexpr int LCD_SPI_INDEX = 0;
static constexpr int PIN_SPI_LCD_CS_N = 17;
static constexpr int PIN_SPI_LCD_SCLK = 18;
static constexpr int PIN_SPI_LCD_MOSI = 19;
static constexpr int PIN_SPI_LCD_RES_N = 20;
static constexpr int PIN_SPI_LCD_DC = 21;
static constexpr uint32_t LCD_SPI_FREQ = 1e6;

//static constexpr int LCD_I2C_INDEX = 0;
//static constexpr int PIN_I2C_LCD_SDA = 12;
//static constexpr int PIN_I2C_LCD_SCL = 13;
static constexpr int LCD_I2C_INDEX = 1;
static constexpr int PIN_I2C_LCD_SDA = 14;
static constexpr int PIN_I2C_LCD_SCL = 15;
static constexpr int LCD_I2C_ADDR = 0x3c;
static constexpr uint32_t LCD_I2C_FREQ = 400e3;

static constexpr int PIN_ADC_IN = 26;
static constexpr int PIN_SPEAKER_OUT = 28;
static constexpr int PIN_LAMP_OUT = 10;

#ifndef BOARD_PICO_W
static constexpr int PIN_LED_OUT = 25;
#endif

static constexpr uint32_t SPEAKER_FREQ = 440;
static constexpr uint32_t SPEAKER_SAMPLE_BITS = 16;
static constexpr uint32_t SPEAKER_PWM_PERIOD = 1 << SPEAKER_SAMPLE_BITS;

typedef struct {
    shapoco::jjy::rx::Rf::rf_status_t rf;
    shapoco::jjy::rx::sync_status_t sync;
    shapoco::jjy::rx::Decoder::status_t dec;
} receiver_status_t;

extern shapoco::pico::atomic<receiver_status_t> glb_receiver_status;

extern const char * DAY_OF_WEEK_STRING[];

void main(void);

}
