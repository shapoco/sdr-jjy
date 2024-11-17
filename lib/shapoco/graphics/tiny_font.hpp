#pragma once

#include <stdint.h>

namespace shapoco::graphics {

class TinyFontGlyph {
public:
    static constexpr uint8_t BLANK = (1 << 0);
    const uint16_t offset;
    const uint8_t width;
    const uint8_t flags;
    TinyFontGlyph(uint16_t offset, uint8_t width, uint8_t flags = 0) : offset(offset), width(width), flags(flags) { }
    bool isBlank() const { return (flags & BLANK) != 0; }
};

class TinyFont {
public:
    const int height;
    const int codeOffset;
    const int numChars;
    const int spacing;
    const uint8_t *bitmap;
    const TinyFontGlyph *chars;
    
    TinyFont(int height, int codeOffset, int numChars, int spacing, const uint8_t *bitmap, const TinyFontGlyph *chars)
        : height(height), codeOffset(codeOffset), numChars(numChars), spacing(spacing), bitmap(bitmap), chars(chars) { }

    const TinyFontGlyph *getGlyph(char c) const {
        int i = c - codeOffset;
        if ((0 <= i) && (i < numChars)) {
            return &chars[i];
        }
        else {
            return nullptr;
        }
    } 

    int measureStringWidth(const char *s) const {
        const char *c = s;
        int w = 0;
        while (c != nullptr) {
            const TinyFontGlyph *glyph = getGlyph(*c);
            if (glyph) w += glyph->width + spacing;
            c++;
        }
        return w - spacing;
    }
};

}
