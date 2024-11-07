#include <stdint.h>

#include "bmpfont/common.hpp"

namespace bmpfont {

static const uint8_t font4_data[] = {
    0x00, 0x00, 0x00, 0x00, // ' ' (0x20) : x=0, y=0, w=3
    0x00, 0x00, 0x00, 0x00, // '!' (0x21) : x=4, y=0, w=3
    0x00, 0x00, 0x00, 0x00, // '"' (0x22) : x=8, y=0, w=3
    0x00, 0x00, 0x00, 0x00, // '#' (0x23) : x=12, y=0, w=3
    0x00, 0x00, 0x00, 0x00, // '$' (0x24) : x=16, y=0, w=3
    0x00, 0x00, 0x00, 0x00, // '%' (0x25) : x=20, y=0, w=3
    0x00, 0x00, 0x00, 0x00, // '&' (0x26) : x=24, y=0, w=3
    0x00, 0x00, 0x00, 0x00, // ''' (0x27) : x=28, y=0, w=3
    0x00, 0x00, 0x00, 0x00, // '(' (0x28) : x=32, y=0, w=3
    0x00, 0x00, 0x00, 0x00, // ')' (0x29) : x=36, y=0, w=3
    0x00, 0x00, 0x00, 0x00, // '*' (0x2a) : x=40, y=0, w=3
    0x00, 0x00, 0x00, 0x00, // '+' (0x2b) : x=44, y=0, w=3
    0x00, 0x00, 0x00, 0x00, // ',' (0x2c) : x=48, y=0, w=3
    0x00, 0x00, 0x00, 0x00, // '-' (0x2d) : x=52, y=0, w=3
    0x00, 0x00, 0x00, 0x02, // '.' (0x2e) : x=56, y=0, w=3
    0x00, 0x00, 0x00, 0x00, // '/' (0x2f) : x=60, y=0, w=3
    0x00, 0x07, 0x05, 0x07, // '0' (0x30) : x=0, y=6, w=3
    0x03, 0x02, 0x02, 0x02, // '1' (0x31) : x=4, y=6, w=3
    0x00, 0x00, 0x00, 0x00, // '2' (0x32) : x=8, y=6, w=3
    0x00, 0x00, 0x00, 0x00, // '3' (0x33) : x=12, y=6, w=3
    0x00, 0x00, 0x00, 0x00, // '4' (0x34) : x=16, y=6, w=3
    0x00, 0x00, 0x00, 0x00, // '5' (0x35) : x=20, y=6, w=3
    0x00, 0x00, 0x00, 0x00, // '6' (0x36) : x=24, y=6, w=3
    0x00, 0x00, 0x00, 0x00, // '7' (0x37) : x=28, y=6, w=3
    0x00, 0x00, 0x00, 0x00, // '8' (0x38) : x=32, y=6, w=3
    0x00, 0x00, 0x00, 0x00, // '9' (0x39) : x=36, y=6, w=3
    0x00, 0x00, 0x00, 0x00, // ':' (0x3a) : x=40, y=6, w=3
    0x00, 0x00, 0x00, 0x00, // ';' (0x3b) : x=44, y=6, w=3
    0x00, 0x00, 0x00, 0x00, // '<' (0x3c) : x=48, y=6, w=3
    0x00, 0x00, 0x00, 0x00, // '=' (0x3d) : x=52, y=6, w=3
    0x00, 0x00, 0x00, 0x00, // '>' (0x3e) : x=56, y=6, w=3
    0x00, 0x00, 0x00, 0x00, // '?' (0x3f) : x=60, y=6, w=3
    0x00, 0x00, 0x00, 0x00, // '@' (0x40) : x=0, y=12, w=3
    0x00, 0x00, 0x00, 0x00, // 'A' (0x41) : x=4, y=12, w=3
    0x00, 0x00, 0x00, 0x00, // 'B' (0x42) : x=8, y=12, w=3
    0x00, 0x00, 0x00, 0x00, // 'C' (0x43) : x=12, y=12, w=3
    0x00, 0x00, 0x00, 0x00, // 'D' (0x44) : x=16, y=12, w=3
    0x00, 0x00, 0x00, 0x00, // 'E' (0x45) : x=20, y=12, w=3
    0x00, 0x00, 0x00, 0x00, // 'F' (0x46) : x=24, y=12, w=3
    0x00, 0x00, 0x00, 0x00, // 'G' (0x47) : x=28, y=12, w=3
    0x00, 0x00, 0x00, 0x00, // 'H' (0x48) : x=32, y=12, w=3
    0x00, 0x00, 0x00, 0x00, // 'I' (0x49) : x=36, y=12, w=3
    0x00, 0x00, 0x00, 0x00, // 'J' (0x4a) : x=40, y=12, w=3
    0x00, 0x00, 0x00, 0x00, // 'K' (0x4b) : x=44, y=12, w=3
    0x00, 0x00, 0x00, 0x00, // 'L' (0x4c) : x=48, y=12, w=3
    0x07, 0x06, 0x04, 0x00, // 'M' (0x4d) : x=52, y=12, w=3
    0x00, 0x00, 0x00, 0x00, // 'N' (0x4e) : x=56, y=12, w=3
    0x00, 0x00, 0x00, 0x00, // 'O' (0x4f) : x=60, y=12, w=3
    0x00, 0x00, 0x00, 0x00, // 'P' (0x50) : x=0, y=18, w=3
    0x00, 0x00, 0x00, 0x00, // 'Q' (0x51) : x=4, y=18, w=3
    0x00, 0x00, 0x00, 0x00, // 'R' (0x52) : x=8, y=18, w=3
    0x00, 0x00, 0x00, 0x00, // 'S' (0x53) : x=12, y=18, w=3
    0x00, 0x00, 0x00, 0x00, // 'T' (0x54) : x=16, y=18, w=3
    0x00, 0x00, 0x00, 0x00, // 'U' (0x55) : x=20, y=18, w=3
    0x00, 0x00, 0x00, 0x00, // 'V' (0x56) : x=24, y=18, w=3
    0x00, 0x00, 0x00, 0x00, // 'W' (0x57) : x=28, y=18, w=3
    0x05, 0x02, 0x02, 0x05, // 'X' (0x58) : x=32, y=18, w=3
    0x00, 0x00, 0x00, 0x00, // 'Y' (0x59) : x=36, y=18, w=3
    0x00, 0x00, 0x00, 0x00, // 'Z' (0x5a) : x=40, y=18, w=3
    0x00, 0x00, 0x00, 0x00, // '[' (0x5b) : x=44, y=18, w=3
    0x00, 0x00, 0x00, 0x00, // '\' (0x5c) : x=48, y=18, w=3
    0x00, 0x00, 0x00, 0x00, // ']' (0x5d) : x=52, y=18, w=3
    0x00, 0x00, 0x00, 0x00, // '^' (0x5e) : x=56, y=18, w=3
    0x00, 0x00, 0x00, 0x00, // '_' (0x5f) : x=60, y=18, w=3
    0x00, 0x00, 0x00, 0x00, // '`' (0x60) : x=0, y=24, w=3
    0x00, 0x00, 0x00, 0x00, // 'a' (0x61) : x=4, y=24, w=3
    0x00, 0x00, 0x00, 0x00, // 'b' (0x62) : x=8, y=24, w=3
    0x00, 0x00, 0x00, 0x00, // 'c' (0x63) : x=12, y=24, w=3
    0x00, 0x00, 0x00, 0x00, // 'd' (0x64) : x=16, y=24, w=3
    0x00, 0x00, 0x00, 0x00, // 'e' (0x65) : x=20, y=24, w=3
    0x00, 0x00, 0x00, 0x00, // 'f' (0x66) : x=24, y=24, w=3
    0x00, 0x00, 0x00, 0x00, // 'g' (0x67) : x=28, y=24, w=3
    0x00, 0x00, 0x00, 0x00, // 'h' (0x68) : x=32, y=24, w=3
    0x00, 0x00, 0x00, 0x00, // 'i' (0x69) : x=36, y=24, w=3
    0x00, 0x00, 0x00, 0x00, // 'j' (0x6a) : x=40, y=24, w=3
    0x00, 0x00, 0x00, 0x00, // 'k' (0x6b) : x=44, y=24, w=3
    0x00, 0x00, 0x00, 0x00, // 'l' (0x6c) : x=48, y=24, w=3
    0x00, 0x00, 0x00, 0x00, // 'm' (0x6d) : x=52, y=24, w=3
    0x00, 0x00, 0x00, 0x00, // 'n' (0x6e) : x=56, y=24, w=3
    0x00, 0x00, 0x00, 0x00, // 'o' (0x6f) : x=60, y=24, w=3
    0x00, 0x00, 0x00, 0x00, // 'p' (0x70) : x=0, y=30, w=3
    0x00, 0x00, 0x00, 0x00, // 'q' (0x71) : x=4, y=30, w=3
    0x00, 0x00, 0x00, 0x00, // 'r' (0x72) : x=8, y=30, w=3
    0x00, 0x00, 0x00, 0x00, // 's' (0x73) : x=12, y=30, w=3
    0x00, 0x00, 0x00, 0x00, // 't' (0x74) : x=16, y=30, w=3
    0x00, 0x00, 0x00, 0x00, // 'u' (0x75) : x=20, y=30, w=3
    0x00, 0x00, 0x00, 0x00, // 'v' (0x76) : x=24, y=30, w=3
    0x00, 0x00, 0x00, 0x00, // 'w' (0x77) : x=28, y=30, w=3
    0x00, 0x00, 0x00, 0x00, // 'x' (0x78) : x=32, y=30, w=3
    0x00, 0x00, 0x00, 0x00, // 'y' (0x79) : x=36, y=30, w=3
    0x00, 0x00, 0x00, 0x00, // 'z' (0x7a) : x=40, y=30, w=3
    0x00, 0x00, 0x00, 0x00, // '{' (0x7b) : x=44, y=30, w=3
    0x00, 0x00, 0x00, 0x00, // '|' (0x7c) : x=48, y=30, w=3
    0x00, 0x00, 0x00, 0x00, // '}' (0x7d) : x=52, y=30, w=3
    0x00, 0x00, 0x00, 0x00, // '~' (0x7e) : x=56, y=30, w=3
    0x00, 0x00, 0x00, 0x00, // '' (0x7f) : x=60, y=30, w=3
};

static const CharInfo font4_index[] = {
    CharInfo(0, 3), CharInfo(4, 3), CharInfo(8, 3), CharInfo(12, 3), CharInfo(16, 3), CharInfo(20, 3), CharInfo(24, 3), CharInfo(28, 3), 
    CharInfo(32, 3), CharInfo(36, 3), CharInfo(40, 3), CharInfo(44, 3), CharInfo(48, 3), CharInfo(52, 3), CharInfo(56, 3), CharInfo(60, 3), 
    CharInfo(64, 3), CharInfo(68, 3), CharInfo(72, 3), CharInfo(76, 3), CharInfo(80, 3), CharInfo(84, 3), CharInfo(88, 3), CharInfo(92, 3), 
    CharInfo(96, 3), CharInfo(100, 3), CharInfo(104, 3), CharInfo(108, 3), CharInfo(112, 3), CharInfo(116, 3), CharInfo(120, 3), CharInfo(124, 3), 
    CharInfo(128, 3), CharInfo(132, 3), CharInfo(136, 3), CharInfo(140, 3), CharInfo(144, 3), CharInfo(148, 3), CharInfo(152, 3), CharInfo(156, 3), 
    CharInfo(160, 3), CharInfo(164, 3), CharInfo(168, 3), CharInfo(172, 3), CharInfo(176, 3), CharInfo(180, 3), CharInfo(184, 3), CharInfo(188, 3), 
    CharInfo(192, 3), CharInfo(196, 3), CharInfo(200, 3), CharInfo(204, 3), CharInfo(208, 3), CharInfo(212, 3), CharInfo(216, 3), CharInfo(220, 3), 
    CharInfo(224, 3), CharInfo(228, 3), CharInfo(232, 3), CharInfo(236, 3), CharInfo(240, 3), CharInfo(244, 3), CharInfo(248, 3), CharInfo(252, 3), 
    CharInfo(256, 3), CharInfo(260, 3), CharInfo(264, 3), CharInfo(268, 3), CharInfo(272, 3), CharInfo(276, 3), CharInfo(280, 3), CharInfo(284, 3), 
    CharInfo(288, 3), CharInfo(292, 3), CharInfo(296, 3), CharInfo(300, 3), CharInfo(304, 3), CharInfo(308, 3), CharInfo(312, 3), CharInfo(316, 3), 
    CharInfo(320, 3), CharInfo(324, 3), CharInfo(328, 3), CharInfo(332, 3), CharInfo(336, 3), CharInfo(340, 3), CharInfo(344, 3), CharInfo(348, 3), 
    CharInfo(352, 3), CharInfo(356, 3), CharInfo(360, 3), CharInfo(364, 3), CharInfo(368, 3), CharInfo(372, 3), CharInfo(376, 3), CharInfo(380, 3), 
};

Font font4(4, 32, 96, 1, font4_data, font4_index);

}
