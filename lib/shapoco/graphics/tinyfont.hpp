#ifndef BMPFONT_COMMON_HPP
#define BMPFONT_COMMON_HPP

#include <stdint.h>

namespace bmpfont {

class CharInfo {
public:
    static constexpr uint8_t BLANK = (1 << 0);
    const uint16_t offset;
    const uint8_t width;
    const uint8_t flags;
    CharInfo(uint16_t offset, uint8_t width, uint8_t flags = 0) : offset(offset), width(width), flags(flags) { }
    bool isBlank() const { return (flags & BLANK) != 0; }
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
        return (code_offset <= c) && (c < code_offset + num_chars) && (get_char_info(c).width > 0);
    }

    const CharInfo &get_char_info(char c) const {
        return chars[c - code_offset];
    }
};

}

#endif

