#pragma once

#ifndef SHAPOCO_JJY_CFG_RX_ADC_SPS
#define SHAPOCO_JJY_CFG_RX_ADC_SPS (480 * 1000)
#endif

#include <stdint.h>

namespace shapoco::jjy::rx {

static constexpr uint32_t FREQ_60KHZ = 60 * 1000;
static constexpr uint32_t DETECTION_INPUT_SPS = SHAPOCO_JJY_CFG_RX_ADC_SPS;   
static constexpr uint32_t DETECTION_RESOLUTION = SHAPOCO_JJY_CFG_RX_ADC_SPS / FREQ_60KHZ;

static constexpr uint32_t DETECTION_OUTPUT_SPS = 100;
static constexpr uint32_t DETECTION_BLOCK_SIZE = DETECTION_INPUT_SPS / DETECTION_OUTPUT_SPS;

static_assert(DETECTION_RESOLUTION * FREQ_60KHZ == DETECTION_INPUT_SPS);
static_assert(DETECTION_BLOCK_SIZE * DETECTION_OUTPUT_SPS == DETECTION_INPUT_SPS);

}
