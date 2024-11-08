#include <stdint.h>

#include "shapoco/graphics/tiny_font.hpp"

namespace shapoco::jjymon::fonts {

using namespace ::shapoco::graphics;

static const uint8_t font4_data[] = {
    0x00, 0x00, 0x00, 0x02, // '.' (0x2e) : x=56, y=0, w=3
    // (BLANK) '/' (0x2f) : x=60, y=0, w=0
    0x00, 0x07, 0x05, 0x07, // '0' (0x30) : x=0, y=6, w=3
    0x03, 0x02, 0x02, 0x02, // '1' (0x31) : x=4, y=6, w=3
    // (BLANK) '2' (0x32) : x=8, y=6, w=0
    // (BLANK) '3' (0x33) : x=12, y=6, w=0
    // (BLANK) '4' (0x34) : x=16, y=6, w=0
    // (BLANK) '5' (0x35) : x=20, y=6, w=0
    // (BLANK) '6' (0x36) : x=24, y=6, w=0
    // (BLANK) '7' (0x37) : x=28, y=6, w=0
    // (BLANK) '8' (0x38) : x=32, y=6, w=0
    // (BLANK) '9' (0x39) : x=36, y=6, w=0
    // (BLANK) ':' (0x3a) : x=40, y=6, w=0
    // (BLANK) ';' (0x3b) : x=44, y=6, w=0
    // (BLANK) '<' (0x3c) : x=48, y=6, w=0
    // (BLANK) '=' (0x3d) : x=52, y=6, w=0
    // (BLANK) '>' (0x3e) : x=56, y=6, w=0
    // (BLANK) '?' (0x3f) : x=60, y=6, w=0
    // (BLANK) '@' (0x40) : x=0, y=12, w=0
    // (BLANK) 'A' (0x41) : x=4, y=12, w=0
    // (BLANK) 'B' (0x42) : x=8, y=12, w=0
    // (BLANK) 'C' (0x43) : x=12, y=12, w=0
    // (BLANK) 'D' (0x44) : x=16, y=12, w=0
    // (BLANK) 'E' (0x45) : x=20, y=12, w=0
    // (BLANK) 'F' (0x46) : x=24, y=12, w=0
    // (BLANK) 'G' (0x47) : x=28, y=12, w=0
    // (BLANK) 'H' (0x48) : x=32, y=12, w=0
    // (BLANK) 'I' (0x49) : x=36, y=12, w=0
    // (BLANK) 'J' (0x4a) : x=40, y=12, w=0
    // (BLANK) 'K' (0x4b) : x=44, y=12, w=0
    // (BLANK) 'L' (0x4c) : x=48, y=12, w=0
    0x07, 0x06, 0x04, 0x00, // 'M' (0x4d) : x=52, y=12, w=3
    // (BLANK) 'N' (0x4e) : x=56, y=12, w=0
    // (BLANK) 'O' (0x4f) : x=60, y=12, w=0
    // (BLANK) 'P' (0x50) : x=0, y=18, w=0
    // (BLANK) 'Q' (0x51) : x=4, y=18, w=0
    // (BLANK) 'R' (0x52) : x=8, y=18, w=0
    // (BLANK) 'S' (0x53) : x=12, y=18, w=0
    // (BLANK) 'T' (0x54) : x=16, y=18, w=0
    // (BLANK) 'U' (0x55) : x=20, y=18, w=0
    // (BLANK) 'V' (0x56) : x=24, y=18, w=0
    // (BLANK) 'W' (0x57) : x=28, y=18, w=0
    0x05, 0x02, 0x02, 0x05, // 'X' (0x58) : x=32, y=18, w=3
};

static const TinyFontGlyph font4_index[] = {
    TinyFontGlyph(0, 3), 
    TinyFontGlyph(4, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(4, 3), 
    TinyFontGlyph(8, 3), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 3), 
    TinyFontGlyph(16, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(16, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(16, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(16, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(16, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(16, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(16, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(16, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(16, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(16, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(16, 3), 
};

TinyFont font4(4, 46, 43, 1, font4_data, font4_index);

}
