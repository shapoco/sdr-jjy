#ifndef CORE1_HPP
#define CORE1_HPP

constexpr int PIN_BARLED_SEG_BASE = 0;
constexpr int PIN_BARLED_CH_BASE = 10;
constexpr int BARLED_NUM_SEGS = 10;
constexpr int BARLED_NUM_CHS = 3;
constexpr int BARLED_CH_R = 2;
constexpr int BARLED_CH_G = 0;
constexpr int BARLED_CH_B = 1;

void core1_init();
void core1_main();

#endif
