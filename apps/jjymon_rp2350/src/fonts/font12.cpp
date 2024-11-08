#include <stdint.h>

#include "shapoco/graphics/tiny_font.hpp"

namespace shapoco::jjymon::fonts {

using namespace ::shapoco::graphics;

static const uint8_t font12_data[] = {
    // (BLANK) ' ' (0x20) : x=0, y=0, w=4
    // (BLANK) '!' (0x21) : x=5, y=0, w=0
    // (BLANK) '"' (0x22) : x=9, y=0, w=0
    // (BLANK) '#' (0x23) : x=13, y=0, w=0
    // (BLANK) '$' (0x24) : x=17, y=0, w=0
    // (BLANK) '%' (0x25) : x=21, y=0, w=0
    // (BLANK) '&' (0x26) : x=25, y=0, w=0
    // (BLANK) ''' (0x27) : x=29, y=0, w=0
    // (BLANK) '(' (0x28) : x=33, y=0, w=0
    // (BLANK) ')' (0x29) : x=37, y=0, w=0
    // (BLANK) '*' (0x2a) : x=41, y=0, w=0
    // (BLANK) '+' (0x2b) : x=45, y=0, w=0
    // (BLANK) ',' (0x2c) : x=49, y=0, w=0
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, // '-' (0x2d) : x=53, y=0, w=6
    // (BLANK) '.' (0x2e) : x=60, y=0, w=0
    0x00, 0x30, 0x30, 0x18, 0x18, 0x0c, 0x0c, 0x0c, 0x06, 0x06, 0x03, 0x03, // '/' (0x2f) : x=64, y=0, w=6
    0x00, 0x1e, 0x3f, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3f, 0x1e, // '0' (0x30) : x=0, y=14, w=6
    0x00, 0x0c, 0x0e, 0x0e, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, // '1' (0x31) : x=7, y=14, w=6
    0x00, 0x1e, 0x3f, 0x33, 0x30, 0x18, 0x1c, 0x0e, 0x06, 0x07, 0x3f, 0x3f, // '2' (0x32) : x=14, y=14, w=6
    0x00, 0x3f, 0x3f, 0x18, 0x0c, 0x1e, 0x3e, 0x30, 0x30, 0x33, 0x3f, 0x1e, // '3' (0x33) : x=21, y=14, w=6
    0x00, 0x0c, 0x0c, 0x0c, 0x06, 0x06, 0x1b, 0x1b, 0x3f, 0x3f, 0x18, 0x18, // '4' (0x34) : x=28, y=14, w=6
    0x00, 0x3f, 0x3f, 0x03, 0x03, 0x1f, 0x3f, 0x30, 0x30, 0x33, 0x3f, 0x1e, // '5' (0x35) : x=35, y=14, w=6
    0x00, 0x0c, 0x0c, 0x06, 0x06, 0x1f, 0x3f, 0x33, 0x33, 0x33, 0x3f, 0x1e, // '6' (0x36) : x=42, y=14, w=6
    0x00, 0x3f, 0x3f, 0x33, 0x30, 0x18, 0x18, 0x18, 0x0c, 0x0c, 0x0c, 0x0c, // '7' (0x37) : x=49, y=14, w=6
    0x00, 0x1e, 0x3f, 0x33, 0x33, 0x1e, 0x1e, 0x33, 0x33, 0x33, 0x3f, 0x1e, // '8' (0x38) : x=56, y=14, w=6
    0x00, 0x1e, 0x3f, 0x33, 0x33, 0x33, 0x3f, 0x3e, 0x18, 0x18, 0x0c, 0x0c, // '9' (0x39) : x=63, y=14, w=6
    0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, // ':' (0x3a) : x=70, y=14, w=2
    // (BLANK) ';' (0x3b) : x=73, y=14, w=0
    // (BLANK) '<' (0x3c) : x=77, y=14, w=0
    // (BLANK) '=' (0x3d) : x=81, y=14, w=0
    // (BLANK) '>' (0x3e) : x=85, y=14, w=0
    0x00, 0x1e, 0x3f, 0x33, 0x30, 0x38, 0x1c, 0x0c, 0x0c, 0x00, 0x0c, 0x0c, // '?' (0x3f) : x=89, y=14, w=6
};

static const TinyFontGlyph font12_index[] = {
    TinyFontGlyph(0, 4, TinyFontGlyph::BLANK), 
    TinyFontGlyph(0, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(0, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(0, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(0, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(0, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(0, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(0, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(0, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(0, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(0, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(0, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(0, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(0, 6), 
    TinyFontGlyph(12, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(12, 6), 
    TinyFontGlyph(24, 6), 
    TinyFontGlyph(36, 6), 
    TinyFontGlyph(48, 6), 
    TinyFontGlyph(60, 6), 
    TinyFontGlyph(72, 6), 
    TinyFontGlyph(84, 6), 
    TinyFontGlyph(96, 6), 
    TinyFontGlyph(108, 6), 
    TinyFontGlyph(120, 6), 
    TinyFontGlyph(132, 6), 
    TinyFontGlyph(144, 2), 
    TinyFontGlyph(156, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(156, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(156, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(156, 0, TinyFontGlyph::BLANK), 
    TinyFontGlyph(156, 6), 
};

TinyFont font12(12, 32, 32, 1, font12_data, font12_index);

}
