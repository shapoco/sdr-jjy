#ifndef METER_HPP
#define METER_HPP

#include "jjymon.hpp"
#include "shapoco/fixed12.hpp"
#include "shapoco/pico/ssd1309spi.hpp"
#include "images.hpp"

using pen_t = ssd1309spi::pen_t;

class Meter {
public:
    static constexpr int WIDTH = 32;
    static constexpr int HEIGHT = 12;

private:
    static constexpr int WAVEFORM_PERIOD = 1 << 7;

    int32_t waveform_max[WAVEFORM_PERIOD];
    int32_t waveform_min[WAVEFORM_PERIOD];

    int32_t disp_phase_offset = 0;
    int cursor_phase = 0;

public:
};

#endif
