#include <stdio.h>

#include "hardware/clocks.h"
#include "pico/stdlib.h"

#include "shapoco/pico/atomic.hpp"

#include "jjymon.hpp"
#include "sdr_loop.hpp"
#include "ui.hpp"

int main(void) {
    shapoco::jjymon::main();
    return 0;
}

namespace shapoco::jjymon {

::shapoco::pico::atomic<receiver_status_t> glb_receiver_status;

void main(void) {
    // クロック設定
    set_sys_clock_khz(shapoco::jjymon::SYS_CLK_FREQ / KHZ, true);
    sleep_ms(100);

#if ENABLE_STDOUT
    // 標準出力初期化
    stdio_init_all();
    sleep_ms(500);
    printf("Start.\n");
#endif

    // 固定小数点ライブラリの初期化
    fxp12::init_tables();

    // UI初期設定
    ui_init();

    // SDR初期設定
    sdr_init();

    // UIループ起動
    multicore_launch_core1(ui_loop);

    // SDRループ起動
    sdr_loop();
}

}
