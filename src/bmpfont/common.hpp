#ifndef BMPFONT_COMMON_HPP
#define BMPFONT_COMMON_HPP

#include <stdint.h>

namespace bmpfont {

class CharInfo {
public:
    const uint16_t offset;
    const uint8_t width;
    
    CharInfo(uint16_t offset, uint8_t width) : offset(offset), width(width) { }
};

class Font {
public:
    const int height;
    const int code_offset;
    const int num_chars;
    const int spacing;
    const uint8_t *bitmap;
    const CharInfo *chars;
    
    Font(int height, int code_offset, int num_chars, int spacing, const uint8_t *bitmap, const CharInfo *chars)
        : height(height), code_offset(code_offset), num_chars(num_chars), spacing(spacing), bitmap(bitmap), chars(chars) { }
    
    bool contains_char(char c) const {
        return (code_offset <= c && c < code_offset + num_chars);
    }

    const CharInfo &get_char_info(char c) const {
        return chars[c - code_offset];
    }
};

}

#endif

