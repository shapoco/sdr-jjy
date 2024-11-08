#include <stdint.h>

#include "bmpfont/common.hpp"

namespace bmpfont {

static const uint8_t font5_data[] = {
    // (BLANK) ' ' (0x20) : x=0, y=0, w=3
    0x01, 0x01, 0x01, 0x00, 0x01, // '!' (0x21) : x=4, y=0, w=1
    0x05, 0x05, 0x00, 0x00, 0x00, // '"' (0x22) : x=6, y=0, w=3
    0x0a, 0x1f, 0x0a, 0x1f, 0x0a, // '#' (0x23) : x=10, y=0, w=5
    0x0e, 0x05, 0x0e, 0x14, 0x0e, // '$' (0x24) : x=16, y=0, w=5
    0x17, 0x0f, 0x04, 0x1e, 0x19, // '%' (0x25) : x=22, y=0, w=5
    0x07, 0x15, 0x16, 0x09, 0x16, // '&' (0x26) : x=28, y=0, w=5
    0x01, 0x01, 0x00, 0x00, 0x00, // ''' (0x27) : x=34, y=0, w=1
    0x02, 0x01, 0x01, 0x01, 0x02, // '(' (0x28) : x=36, y=0, w=2
    0x01, 0x02, 0x02, 0x02, 0x01, // ')' (0x29) : x=39, y=0, w=2
    0x04, 0x15, 0x0e, 0x15, 0x04, // '*' (0x2a) : x=42, y=0, w=5
    0x04, 0x04, 0x1f, 0x04, 0x04, // '+' (0x2b) : x=48, y=0, w=5
    0x00, 0x00, 0x00, 0x01, 0x01, // ',' (0x2c) : x=54, y=0, w=1
    0x00, 0x00, 0x07, 0x00, 0x00, // '-' (0x2d) : x=56, y=0, w=3
    0x00, 0x00, 0x00, 0x00, 0x01, // '.' (0x2e) : x=60, y=0, w=1
    0x10, 0x08, 0x04, 0x02, 0x01, // '/' (0x2f) : x=62, y=0, w=5
    0x06, 0x09, 0x09, 0x09, 0x06, // '0' (0x30) : x=0, y=7, w=4
    0x04, 0x06, 0x04, 0x04, 0x04, // '1' (0x31) : x=5, y=7, w=4
    0x06, 0x09, 0x04, 0x02, 0x0f, // '2' (0x32) : x=10, y=7, w=4
    0x07, 0x08, 0x07, 0x08, 0x07, // '3' (0x33) : x=15, y=7, w=4
    0x0c, 0x0a, 0x09, 0x0f, 0x08, // '4' (0x34) : x=20, y=7, w=4
    0x0f, 0x01, 0x07, 0x08, 0x07, // '5' (0x35) : x=25, y=7, w=4
    0x06, 0x01, 0x07, 0x09, 0x06, // '6' (0x36) : x=30, y=7, w=4
    0x0f, 0x09, 0x08, 0x04, 0x04, // '7' (0x37) : x=35, y=7, w=4
    0x06, 0x09, 0x06, 0x09, 0x06, // '8' (0x38) : x=40, y=7, w=4
    0x06, 0x09, 0x0e, 0x08, 0x06, // '9' (0x39) : x=45, y=7, w=4
    0x00, 0x01, 0x00, 0x00, 0x01, // ':' (0x3a) : x=50, y=7, w=1
    0x00, 0x01, 0x00, 0x01, 0x01, // ';' (0x3b) : x=52, y=7, w=1
    0x04, 0x02, 0x01, 0x02, 0x04, // '<' (0x3c) : x=54, y=7, w=3
    0x00, 0x03, 0x00, 0x03, 0x00, // '=' (0x3d) : x=58, y=7, w=2
    0x01, 0x02, 0x04, 0x02, 0x01, // '>' (0x3e) : x=61, y=7, w=3
    0x03, 0x04, 0x02, 0x00, 0x02, // '?' (0x3f) : x=65, y=7, w=3
    0x06, 0x0d, 0x0d, 0x01, 0x0e, // '@' (0x40) : x=0, y=14, w=4
    0x06, 0x09, 0x09, 0x0f, 0x09, // 'A' (0x41) : x=5, y=14, w=4
    0x07, 0x09, 0x07, 0x09, 0x07, // 'B' (0x42) : x=10, y=14, w=4
    0x06, 0x01, 0x01, 0x01, 0x06, // 'C' (0x43) : x=15, y=14, w=3
    0x07, 0x09, 0x09, 0x09, 0x07, // 'D' (0x44) : x=19, y=14, w=4
    0x07, 0x01, 0x07, 0x01, 0x07, // 'E' (0x45) : x=24, y=14, w=3
    0x07, 0x01, 0x07, 0x01, 0x01, // 'F' (0x46) : x=28, y=14, w=3
    0x06, 0x01, 0x0d, 0x09, 0x0e, // 'G' (0x47) : x=32, y=14, w=4
    0x09, 0x09, 0x0f, 0x09, 0x09, // 'H' (0x48) : x=37, y=14, w=4
    0x01, 0x01, 0x01, 0x01, 0x01, // 'I' (0x49) : x=42, y=14, w=1
    0x08, 0x08, 0x08, 0x09, 0x06, // 'J' (0x4a) : x=44, y=14, w=4
    0x09, 0x05, 0x03, 0x05, 0x09, // 'K' (0x4b) : x=49, y=14, w=4
    0x01, 0x01, 0x01, 0x01, 0x07, // 'L' (0x4c) : x=54, y=14, w=3
    0x11, 0x1b, 0x15, 0x11, 0x11, // 'M' (0x4d) : x=58, y=14, w=5
    0x09, 0x0b, 0x0d, 0x09, 0x09, // 'N' (0x4e) : x=64, y=14, w=4
    0x0e, 0x11, 0x11, 0x11, 0x0e, // 'O' (0x4f) : x=69, y=14, w=5
    0x07, 0x09, 0x07, 0x01, 0x01, // 'P' (0x50) : x=0, y=21, w=4
    0x0e, 0x11, 0x15, 0x19, 0x1e, // 'Q' (0x51) : x=5, y=21, w=5
    0x07, 0x09, 0x07, 0x09, 0x09, // 'R' (0x52) : x=11, y=21, w=4
    0x0e, 0x01, 0x06, 0x08, 0x07, // 'S' (0x53) : x=16, y=21, w=4
    0x07, 0x02, 0x02, 0x02, 0x02, // 'T' (0x54) : x=21, y=21, w=3
    0x09, 0x09, 0x09, 0x09, 0x06, // 'U' (0x55) : x=25, y=21, w=4
    0x11, 0x11, 0x11, 0x0a, 0x04, // 'V' (0x56) : x=30, y=21, w=5
    0x11, 0x15, 0x15, 0x1f, 0x0a, // 'W' (0x57) : x=36, y=21, w=5
    0x11, 0x0a, 0x04, 0x0a, 0x11, // 'X' (0x58) : x=42, y=21, w=5
    0x11, 0x11, 0x0a, 0x04, 0x04, // 'Y' (0x59) : x=48, y=21, w=5
    0x0f, 0x04, 0x02, 0x01, 0x0f, // 'Z' (0x5a) : x=54, y=21, w=4
    0x03, 0x01, 0x01, 0x01, 0x03, // '[' (0x5b) : x=59, y=21, w=2
    0x01, 0x02, 0x04, 0x08, 0x10, // '\' (0x5c) : x=62, y=21, w=5
    0x03, 0x02, 0x02, 0x02, 0x03, // ']' (0x5d) : x=68, y=21, w=2
    0x02, 0x05, 0x00, 0x00, 0x00, // '^' (0x5e) : x=71, y=21, w=3
    0x00, 0x00, 0x00, 0x00, 0x07, // '_' (0x5f) : x=75, y=21, w=3
    0x01, 0x02, 0x00, 0x00, 0x00, // '`' (0x60) : x=0, y=28, w=2
    0x00, 0x06, 0x08, 0x0f, 0x0f, // 'a' (0x61) : x=3, y=28, w=4
    0x01, 0x07, 0x09, 0x09, 0x07, // 'b' (0x62) : x=8, y=28, w=4
    0x00, 0x06, 0x01, 0x01, 0x06, // 'c' (0x63) : x=13, y=28, w=3
    0x08, 0x0e, 0x09, 0x09, 0x0e, // 'd' (0x64) : x=17, y=28, w=4
    0x00, 0x06, 0x0f, 0x01, 0x06, // 'e' (0x65) : x=22, y=28, w=4
    0x06, 0x01, 0x07, 0x01, 0x01, // 'f' (0x66) : x=27, y=28, w=3
    0x00, 0x0f, 0x0f, 0x08, 0x06, // 'g' (0x67) : x=31, y=28, w=4
    0x01, 0x01, 0x07, 0x09, 0x09, // 'h' (0x68) : x=36, y=28, w=4
    0x01, 0x00, 0x01, 0x01, 0x01, // 'i' (0x69) : x=41, y=28, w=1
    0x04, 0x00, 0x04, 0x04, 0x03, // 'j' (0x6a) : x=43, y=28, w=3
    0x01, 0x09, 0x05, 0x07, 0x09, // 'k' (0x6b) : x=47, y=28, w=4
    0x03, 0x02, 0x02, 0x02, 0x02, // 'l' (0x6c) : x=52, y=28, w=2
    0x00, 0x0b, 0x15, 0x15, 0x15, // 'm' (0x6d) : x=55, y=28, w=5
    0x00, 0x07, 0x09, 0x09, 0x09, // 'n' (0x6e) : x=61, y=28, w=4
    0x00, 0x06, 0x09, 0x09, 0x06, // 'o' (0x6f) : x=66, y=28, w=4
    0x00, 0x07, 0x09, 0x07, 0x01, // 'p' (0x70) : x=0, y=35, w=4
    0x00, 0x0e, 0x09, 0x0e, 0x08, // 'q' (0x71) : x=5, y=35, w=4
    0x00, 0x05, 0x03, 0x01, 0x01, // 'r' (0x72) : x=10, y=35, w=3
    0x00, 0x0e, 0x03, 0x0c, 0x07, // 's' (0x73) : x=14, y=35, w=4
    0x01, 0x07, 0x01, 0x01, 0x06, // 't' (0x74) : x=19, y=35, w=3
    0x00, 0x09, 0x09, 0x09, 0x0e, // 'u' (0x75) : x=23, y=35, w=4
    0x00, 0x11, 0x11, 0x0a, 0x04, // 'v' (0x76) : x=28, y=35, w=5
    0x00, 0x11, 0x15, 0x1f, 0x0a, // 'w' (0x77) : x=34, y=35, w=5
    0x00, 0x09, 0x06, 0x06, 0x09, // 'x' (0x78) : x=40, y=35, w=4
    0x00, 0x09, 0x0e, 0x08, 0x06, // 'y' (0x79) : x=45, y=35, w=4
    0x00, 0x0f, 0x04, 0x02, 0x0f, // 'z' (0x7a) : x=50, y=35, w=4
    0x06, 0x02, 0x03, 0x02, 0x06, // '{' (0x7b) : x=55, y=35, w=3
    0x01, 0x01, 0x01, 0x01, 0x01, // '|' (0x7c) : x=59, y=35, w=1
    0x03, 0x02, 0x06, 0x02, 0x03, // '}' (0x7d) : x=61, y=35, w=3
    0x00, 0x02, 0x15, 0x08, 0x00, // '~' (0x7e) : x=65, y=35, w=5
};

static const CharInfo font5_index[] = {
    CharInfo(0, 3, CharInfo::BLANK), 
    CharInfo(0, 1), 
    CharInfo(5, 3), 
    CharInfo(10, 5), 
    CharInfo(15, 5), 
    CharInfo(20, 5), 
    CharInfo(25, 5), 
    CharInfo(30, 1), 
    CharInfo(35, 2), 
    CharInfo(40, 2), 
    CharInfo(45, 5), 
    CharInfo(50, 5), 
    CharInfo(55, 1), 
    CharInfo(60, 3), 
    CharInfo(65, 1), 
    CharInfo(70, 5), 
    CharInfo(75, 4), 
    CharInfo(80, 4), 
    CharInfo(85, 4), 
    CharInfo(90, 4), 
    CharInfo(95, 4), 
    CharInfo(100, 4), 
    CharInfo(105, 4), 
    CharInfo(110, 4), 
    CharInfo(115, 4), 
    CharInfo(120, 4), 
    CharInfo(125, 1), 
    CharInfo(130, 1), 
    CharInfo(135, 3), 
    CharInfo(140, 2), 
    CharInfo(145, 3), 
    CharInfo(150, 3), 
    CharInfo(155, 4), 
    CharInfo(160, 4), 
    CharInfo(165, 4), 
    CharInfo(170, 3), 
    CharInfo(175, 4), 
    CharInfo(180, 3), 
    CharInfo(185, 3), 
    CharInfo(190, 4), 
    CharInfo(195, 4), 
    CharInfo(200, 1), 
    CharInfo(205, 4), 
    CharInfo(210, 4), 
    CharInfo(215, 3), 
    CharInfo(220, 5), 
    CharInfo(225, 4), 
    CharInfo(230, 5), 
    CharInfo(235, 4), 
    CharInfo(240, 5), 
    CharInfo(245, 4), 
    CharInfo(250, 4), 
    CharInfo(255, 3), 
    CharInfo(260, 4), 
    CharInfo(265, 5), 
    CharInfo(270, 5), 
    CharInfo(275, 5), 
    CharInfo(280, 5), 
    CharInfo(285, 4), 
    CharInfo(290, 2), 
    CharInfo(295, 5), 
    CharInfo(300, 2), 
    CharInfo(305, 3), 
    CharInfo(310, 3), 
    CharInfo(315, 2), 
    CharInfo(320, 4), 
    CharInfo(325, 4), 
    CharInfo(330, 3), 
    CharInfo(335, 4), 
    CharInfo(340, 4), 
    CharInfo(345, 3), 
    CharInfo(350, 4), 
    CharInfo(355, 4), 
    CharInfo(360, 1), 
    CharInfo(365, 3), 
    CharInfo(370, 4), 
    CharInfo(375, 2), 
    CharInfo(380, 5), 
    CharInfo(385, 4), 
    CharInfo(390, 4), 
    CharInfo(395, 4), 
    CharInfo(400, 4), 
    CharInfo(405, 3), 
    CharInfo(410, 4), 
    CharInfo(415, 3), 
    CharInfo(420, 4), 
    CharInfo(425, 5), 
    CharInfo(430, 5), 
    CharInfo(435, 4), 
    CharInfo(440, 4), 
    CharInfo(445, 4), 
    CharInfo(450, 3), 
    CharInfo(455, 1), 
    CharInfo(460, 3), 
    CharInfo(465, 5), 
};

Font font5(5, 32, 95, 1, font5_data, font5_index);

}