#ifndef BIT_LOG_TABLE_HPP
#define BIT_LOG_TABLE_HPP

#include "jjymon.hpp"
#include "jjy/jjy.hpp"
#include "fixed12.hpp"
#include "ssd1309spi.hpp"
#include "images/images.hpp"
#include "bmpfont/bmpfont.hpp"

typedef struct  {
    bool valid;
    jjy::jjybit_t value;
} bit_cell_t;

class BitLogTable {
public:
    static constexpr int COLS = 10;
    static constexpr int ROWS = 7;

    
};

#endif
