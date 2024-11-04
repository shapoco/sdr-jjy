#ifndef LCD_DEMO_HPP
#define LCD_DEMO_HPP

#include <stdint.h>

#include "jjymon.hpp"
#include "fixed12.hpp"

void lcd_demo_render(JjyLcd &lcd, uint32_t t_now_ms);

#endif
