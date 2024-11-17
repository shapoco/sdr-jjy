#include <stdint.h>

#include "shapoco/graphics/tiny_font.hpp"

namespace shapoco::jjymon::fonts {

using namespace ::shapoco::graphics;

static const uint8_t font4_data[] = {
    0x00, 0x00, 0x00, 0x02, // '.' (0x2e) : index=0, w=3
    // (BLANK) '/' (0x2f) : index=4, w=0
    0x00, 0x07, 0x05, 0x07, // '0' (0x30) : index=4, w=3
    0x03, 0x02, 0x02, 0x02, // '1' (0x31) : index=8, w=3
    // (BLANK) '2' (0x32) : index=12, w=0
    // (BLANK) '3' (0x33) : index=12, w=0
    // (BLANK) '4' (0x34) : index=12, w=0
    // (BLANK) '5' (0x35) : index=12, w=0
    // (BLANK) '6' (0x36) : index=12, w=0
    // (BLANK) '7' (0x37) : index=12, w=0
    // (BLANK) '8' (0x38) : index=12, w=0
    // (BLANK) '9' (0x39) : index=12, w=0
    // (BLANK) ':' (0x3a) : index=12, w=0
    // (BLANK) ';' (0x3b) : index=12, w=0
    // (BLANK) '<' (0x3c) : index=12, w=0
    // (BLANK) '=' (0x3d) : index=12, w=0
    // (BLANK) '>' (0x3e) : index=12, w=0
    // (BLANK) '?' (0x3f) : index=12, w=0
    // (BLANK) '@' (0x40) : index=12, w=0
    // (BLANK) 'A' (0x41) : index=12, w=0
    // (BLANK) 'B' (0x42) : index=12, w=0
    // (BLANK) 'C' (0x43) : index=12, w=0
    // (BLANK) 'D' (0x44) : index=12, w=0
    // (BLANK) 'E' (0x45) : index=12, w=0
    // (BLANK) 'F' (0x46) : index=12, w=0
    // (BLANK) 'G' (0x47) : index=12, w=0
    // (BLANK) 'H' (0x48) : index=12, w=0
    // (BLANK) 'I' (0x49) : index=12, w=0
    // (BLANK) 'J' (0x4a) : index=12, w=0
    // (BLANK) 'K' (0x4b) : index=12, w=0
    // (BLANK) 'L' (0x4c) : index=12, w=0
    0x07, 0x06, 0x04, 0x00, // 'M' (0x4d) : index=12, w=3
    // (BLANK) 'N' (0x4e) : index=16, w=0
    // (BLANK) 'O' (0x4f) : index=16, w=0
    // (BLANK) 'P' (0x50) : index=16, w=0
    // (BLANK) 'Q' (0x51) : index=16, w=0
    // (BLANK) 'R' (0x52) : index=16, w=0
    // (BLANK) 'S' (0x53) : index=16, w=0
    // (BLANK) 'T' (0x54) : index=16, w=0
    // (BLANK) 'U' (0x55) : index=16, w=0
    // (BLANK) 'V' (0x56) : index=16, w=0
    // (BLANK) 'W' (0x57) : index=16, w=0
    0x05, 0x02, 0x02, 0x05, // 'X' (0x58) : index=16, w=3
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
