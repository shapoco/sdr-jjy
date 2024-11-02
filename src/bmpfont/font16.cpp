#include <stdint.h>

#include "bmpfont/common.hpp"

namespace bmpfont {

static const uint8_t font16_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ' ' (0x20) : x=0, y=0, w=3
    0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, // '!' (0x21) : x=4, y=0, w=2
    0x77, 0x77, 0x77, 0x66, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // '"' (0x22) : x=7, y=0, w=7
    0x00, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xff, 0x01, 0xff, 0x01, 0xcc, 0x00, 0xcc, 0x00, 0x66, 0x00, // '#' (0x23) : x=15, y=0, w=9
    0x66, 0x00, 0xff, 0x01, 0xff, 0x01, 0x66, 0x00, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x18, 0x7e, 0x7f, 0x1b, 0x1b, 0x7f, 0xfe, 0xd8, 0xd8, 0xfe, 0x7e, 0x18, 0x00, 0x00, 0x00, // '$' (0x24) : x=25, y=0, w=8
    0x00, 0x00, 0x1e, 0x03, 0x3f, 0x03, 0xb3, 0x01, 0xf3, 0x01, 0xf3, 0x00, 0xff, 0x0f, 0xfe, 0x1f, // '%' (0x25) : x=34, y=0, w=13
    0xe0, 0x19, 0xb0, 0x19, 0xb0, 0x19, 0x98, 0x1f, 0x18, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x7c, 0x00, 0xfe, 0x00, 0xc6, 0x00, 0xc6, 0x00, 0xee, 0x00, 0x7c, 0x00, 0x3e, 0x00, // '&' (0x26) : x=48, y=0, w=10
    0x77, 0x03, 0xe3, 0x03, 0xc3, 0x01, 0xff, 0x03, 0x7e, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x07, 0x07, 0x07, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ''' (0x27) : x=59, y=0, w=3
    0x0c, 0x06, 0x06, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x06, 0x06, 0x0c, 0x00, 0x00, // '(' (0x28) : x=63, y=0, w=4
    0x03, 0x06, 0x06, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x06, 0x06, 0x03, 0x00, 0x00, // ')' (0x29) : x=68, y=0, w=4
    0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x30, 0x00, 0x33, 0x03, 0xff, 0x03, 0xfc, 0x00, 0x30, 0x00, // '*' (0x2a) : x=73, y=0, w=10
    0xfc, 0x00, 0xff, 0x03, 0x33, 0x03, 0x30, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0xff, 0xff, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, // '+' (0x2b) : x=84, y=0, w=8
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x06, 0x03, 0x00, // ',' (0x2c) : x=93, y=0, w=3
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // '-' (0x2d) : x=97, y=0, w=7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, // '.' (0x2e) : x=105, y=0, w=3
    0xc0, 0xc0, 0x60, 0x60, 0x30, 0x30, 0x18, 0x18, 0x0c, 0x0c, 0x06, 0x06, 0x03, 0x03, 0x00, 0x00, // '/' (0x2f) : x=109, y=0, w=8
    0x00, 0x7e, 0xff, 0xc3, 0xc3, 0xc7, 0xcb, 0xd3, 0xe3, 0xc3, 0xc3, 0xff, 0x7e, 0x00, 0x00, 0x00, // '0' (0x30) : x=0, y=18, w=8
    0x00, 0x30, 0x38, 0x3c, 0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, // '1' (0x31) : x=9, y=18, w=8
    0x00, 0x7e, 0xff, 0xc3, 0xc3, 0xe0, 0x70, 0x38, 0x1c, 0x0e, 0x07, 0xff, 0xff, 0x00, 0x00, 0x00, // '2' (0x32) : x=18, y=18, w=8
    0x00, 0xff, 0xff, 0x70, 0x38, 0x7c, 0xfc, 0xc0, 0xc0, 0xc3, 0xc3, 0xff, 0x7e, 0x00, 0x00, 0x00, // '3' (0x33) : x=27, y=18, w=8
    0x00, 0x0c, 0x0c, 0x0c, 0x06, 0x66, 0x66, 0x63, 0xff, 0xff, 0x60, 0x60, 0x60, 0x00, 0x00, 0x00, // '4' (0x34) : x=36, y=18, w=8
    0x00, 0xff, 0xff, 0x03, 0x03, 0x7f, 0xff, 0xc0, 0xc0, 0xc3, 0xc3, 0xff, 0x7e, 0x00, 0x00, 0x00, // '5' (0x35) : x=45, y=18, w=8
    0x00, 0x7e, 0xff, 0xc3, 0x03, 0x7f, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0x7e, 0x00, 0x00, 0x00, // '6' (0x36) : x=54, y=18, w=8
    0x00, 0xff, 0xff, 0xc3, 0x63, 0x60, 0x60, 0x30, 0x30, 0x30, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, // '7' (0x37) : x=63, y=18, w=8
    0x00, 0x7e, 0xff, 0xc3, 0xc3, 0xc3, 0x7e, 0x7e, 0xc3, 0xc3, 0xc3, 0xff, 0x7e, 0x00, 0x00, 0x00, // '8' (0x38) : x=72, y=18, w=8
    0x00, 0x7e, 0xff, 0xc3, 0xc3, 0xc3, 0xff, 0xfe, 0xc0, 0xc0, 0xc3, 0xff, 0x7e, 0x00, 0x00, 0x00, // '9' (0x39) : x=81, y=18, w=8
    0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, // ':' (0x3a) : x=90, y=18, w=3
    0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x06, 0x03, 0x00, // ';' (0x3b) : x=94, y=18, w=3
    0x00, 0x00, 0x20, 0x30, 0x38, 0x1c, 0x0e, 0x07, 0x0e, 0x1c, 0x38, 0x30, 0x20, 0x00, 0x00, 0x00, // '<' (0x3c) : x=98, y=18, w=6
    0x00, 0x00, 0x00, 0x00, 0x3f, 0x3f, 0x00, 0x00, 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // '=' (0x3d) : x=105, y=18, w=6
    0x00, 0x00, 0x01, 0x03, 0x07, 0x0e, 0x1c, 0x38, 0x1c, 0x0e, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, // '>' (0x3e) : x=112, y=18, w=6
    0x00, 0x7e, 0xff, 0xc3, 0xc3, 0xe0, 0x70, 0x38, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, // '?' (0x3f) : x=119, y=18, w=8
    0x00, 0x00, 0xfe, 0x01, 0xff, 0x03, 0x03, 0x03, 0xf3, 0x03, 0xfb, 0x03, 0x1b, 0x03, 0x1b, 0x03, // '@' (0x40) : x=0, y=36, w=10
    0xfb, 0x03, 0xf3, 0x03, 0x03, 0x00, 0xff, 0x01, 0xfe, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x30, 0x00, 0x78, 0x00, 0xfc, 0x00, 0xce, 0x01, 0x87, 0x03, 0x03, 0x03, 0x03, 0x03, // 'A' (0x41) : x=11, y=36, w=10
    0xff, 0x03, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0xff, 0x00, 0xff, 0x01, 0x83, 0x01, 0x83, 0x01, 0x83, 0x01, 0xff, 0x00, 0xff, 0x01, // 'B' (0x42) : x=22, y=36, w=10
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xff, 0x03, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0xfe, 0x01, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, // 'C' (0x43) : x=33, y=36, w=10
    0x03, 0x00, 0x03, 0x03, 0x03, 0x03, 0xff, 0x03, 0xfe, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x7f, 0x00, 0xff, 0x00, 0xc3, 0x01, 0x83, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, // 'D' (0x44) : x=44, y=36, w=10
    0x03, 0x03, 0x83, 0x03, 0xc3, 0x01, 0xff, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0xff, 0xff, 0x03, 0x03, 0x03, 0x7f, 0x7f, 0x03, 0x03, 0x03, 0xff, 0xff, 0x00, 0x00, 0x00, // 'E' (0x45) : x=55, y=36, w=8
    0x00, 0xff, 0xff, 0x03, 0x03, 0x03, 0x7f, 0x7f, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, // 'F' (0x46) : x=64, y=36, w=8
    0x00, 0x00, 0xfe, 0x01, 0xff, 0x01, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0xe3, 0x03, 0xe3, 0x03, // 'G' (0x47) : x=73, y=36, w=10
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xff, 0x03, 0xfe, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xff, 0x03, 0xff, 0x03, // 'H' (0x48) : x=84, y=36, w=10
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, // 'I' (0x49) : x=95, y=36, w=2
    0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, // 'J' (0x4a) : x=98, y=36, w=10
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xff, 0x03, 0xfe, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x83, 0x03, 0xc3, 0x01, 0xe3, 0x00, 0x73, 0x00, 0x3f, 0x00, 0x1f, 0x00, 0x1f, 0x00, // 'K' (0x4b) : x=109, y=36, w=10
    0x3f, 0x00, 0x73, 0x00, 0xe3, 0x00, 0xc3, 0x01, 0x83, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xff, 0xff, 0x00, 0x00, 0x00, // 'L' (0x4c) : x=120, y=36, w=8
    0x00, 0x00, 0x03, 0x0c, 0x07, 0x0e, 0x0f, 0x0f, 0x9f, 0x0f, 0xfb, 0x0d, 0xf3, 0x0c, 0x63, 0x0c, // 'M' (0x4d) : x=129, y=36, w=12
    0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x07, 0x03, 0x0f, 0x03, 0x1f, 0x03, 0x3b, 0x03, 0x73, 0x03, // 'N' (0x4e) : x=142, y=36, w=10
    0xe3, 0x03, 0xc3, 0x03, 0x83, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0xfe, 0x01, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, // 'O' (0x4f) : x=153, y=36, w=10
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xff, 0x03, 0xfe, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0xff, 0x01, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xff, 0x03, // 'P' (0x50) : x=0, y=54, w=10
    0xff, 0x01, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0xfe, 0x01, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, // 'Q' (0x51) : x=11, y=54, w=10
    0x73, 0x03, 0xe3, 0x03, 0xc3, 0x03, 0xff, 0x03, 0xfe, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0xff, 0x01, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xff, 0x03, // 'R' (0x52) : x=22, y=54, w=10
    0xff, 0x01, 0x73, 0x00, 0xe3, 0x00, 0xc3, 0x01, 0x83, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0xfe, 0x01, 0xff, 0x01, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0xff, 0x01, 0xfe, 0x03, // 'S' (0x53) : x=33, y=54, w=10
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0xfe, 0x03, 0xfe, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0xff, 0x03, 0xff, 0x03, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, // 'T' (0x54) : x=44, y=54, w=10
    0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, // 'U' (0x55) : x=55, y=54, w=10
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xff, 0x03, 0xfe, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, // 'V' (0x56) : x=66, y=54, w=10
    0x87, 0x03, 0xce, 0x01, 0xfc, 0x00, 0x78, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x63, 0x0c, 0xf3, 0x0c, // 'W' (0x57) : x=77, y=54, w=12
    0xfb, 0x0d, 0x9f, 0x0f, 0x0f, 0x0f, 0x07, 0x0e, 0x03, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x87, 0x03, 0xce, 0x01, 0xfc, 0x00, 0x78, 0x00, 0x78, 0x00, // 'X' (0x58) : x=90, y=54, w=10
    0xfc, 0x00, 0xce, 0x01, 0x87, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x87, 0x03, 0xce, 0x01, 0xfc, 0x00, 0x78, 0x00, // 'Y' (0x59) : x=101, y=54, w=10
    0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0xff, 0x03, 0xff, 0x03, 0x80, 0x03, 0xc0, 0x01, 0xe0, 0x00, 0x70, 0x00, 0x38, 0x00, // 'Z' (0x5a) : x=112, y=54, w=10
    0x1c, 0x00, 0x0e, 0x00, 0x07, 0x00, 0xff, 0x03, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x0f, 0x0f, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x0f, 0x0f, 0x00, 0x00, // '[' (0x5b) : x=123, y=54, w=4
    0x03, 0x03, 0x06, 0x06, 0x0c, 0x0c, 0x18, 0x18, 0x30, 0x30, 0x60, 0x60, 0xc0, 0xc0, 0x00, 0x00, // '\' (0x5c) : x=128, y=54, w=8
    0x0f, 0x0f, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0f, 0x0f, 0x00, 0x00, // ']' (0x5d) : x=137, y=54, w=4
    0x08, 0x1c, 0x3e, 0x77, 0x63, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // '^' (0x5e) : x=142, y=54, w=7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x7f, 0x00, 0x00, 0x00, // '_' (0x5f) : x=150, y=54, w=7
    0x00, 0x07, 0x07, 0x0e, 0x0e, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // '`' (0x60) : x=0, y=72, w=5
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0xfe, 0xc0, 0xfe, 0xff, 0xc3, 0xff, 0xfe, 0x00, 0x00, 0x00, // 'a' (0x61) : x=6, y=72, w=8
    0x00, 0x03, 0x03, 0x03, 0x03, 0x7f, 0xff, 0xc3, 0xc3, 0xc3, 0xc7, 0xff, 0x7b, 0x00, 0x00, 0x00, // 'b' (0x62) : x=15, y=72, w=8
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0xff, 0xc3, 0x03, 0x03, 0xc3, 0xff, 0x7e, 0x00, 0x00, 0x00, // 'c' (0x63) : x=24, y=72, w=8
    0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xfe, 0xff, 0xc3, 0xc3, 0xc3, 0xe3, 0xff, 0xde, 0x00, 0x00, 0x00, // 'd' (0x64) : x=33, y=72, w=8
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0xff, 0xc3, 0xff, 0xff, 0x03, 0xff, 0x7e, 0x00, 0x00, 0x00, // 'e' (0x65) : x=42, y=72, w=8
    0x00, 0x3c, 0x3e, 0x06, 0x06, 0x3f, 0x3f, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x00, 0x00, 0x00, // 'f' (0x66) : x=51, y=72, w=6
    0x00, 0x00, 0x00, 0x00, 0x00, 0xde, 0xff, 0xe3, 0xc3, 0xc3, 0xff, 0xfe, 0xc0, 0xc0, 0xfe, 0x7e, // 'g' (0x67) : x=58, y=72, w=8
    0x00, 0x03, 0x03, 0x03, 0x03, 0x7b, 0xff, 0xc7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x00, 0x00, 0x00, // 'h' (0x68) : x=67, y=72, w=8
    0x00, 0x00, 0x06, 0x06, 0x00, 0x07, 0x07, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x00, 0x00, 0x00, // 'i' (0x69) : x=76, y=72, w=3
    0x00, 0x00, 0x0c, 0x0c, 0x00, 0x0e, 0x0e, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0f, 0x07, // 'j' (0x6a) : x=80, y=72, w=4
    0x00, 0x03, 0x03, 0x03, 0x03, 0x73, 0x3b, 0x1f, 0x0f, 0x0f, 0x1f, 0x3b, 0x73, 0x00, 0x00, 0x00, // 'k' (0x6b) : x=85, y=72, w=7
    0x00, 0x07, 0x07, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x00, 0x00, 0x00, // 'l' (0x6c) : x=93, y=72, w=3
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbb, 0x01, 0xff, 0x03, 0x77, 0x03, // 'm' (0x6d) : x=97, y=72, w=10
    0x33, 0x03, 0x33, 0x03, 0x33, 0x03, 0x33, 0x03, 0x33, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7b, 0xff, 0xc7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x00, 0x00, 0x00, // 'n' (0x6e) : x=108, y=72, w=8
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0x7e, 0x00, 0x00, 0x00, // 'o' (0x6f) : x=117, y=72, w=8
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7b, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0x7f, 0x03, 0x03, 0x03, // 'p' (0x70) : x=0, y=90, w=8
    0x00, 0x00, 0x00, 0x00, 0x00, 0xde, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xfe, 0xc0, 0xc0, 0xc0, // 'q' (0x71) : x=9, y=90, w=8
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3b, 0x3f, 0x07, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, // 'r' (0x72) : x=18, y=90, w=6
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x7f, 0x03, 0x7f, 0xfe, 0xc0, 0xfe, 0x7e, 0x00, 0x00, 0x00, // 's' (0x73) : x=25, y=90, w=8
    0x00, 0x00, 0x00, 0x06, 0x06, 0x3f, 0x3f, 0x06, 0x06, 0x06, 0x06, 0x3e, 0x3c, 0x00, 0x00, 0x00, // 't' (0x74) : x=34, y=90, w=6
    0x00, 0x00, 0x00, 0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xe3, 0xff, 0xde, 0x00, 0x00, 0x00, // 'u' (0x75) : x=41, y=90, w=8
    0x00, 0x00, 0x00, 0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xe7, 0x7e, 0x3c, 0x18, 0x00, 0x00, 0x00, // 'v' (0x76) : x=50, y=90, w=8
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x03, 0x33, 0x03, 0x33, 0x03, // 'w' (0x77) : x=59, y=90, w=10
    0x33, 0x03, 0x7b, 0x03, 0xff, 0x03, 0xce, 0x01, 0x86, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87, 0x03, 0xce, 0x01, 0xfc, 0x00, // 'x' (0x78) : x=70, y=90, w=11
    0x78, 0x00, 0x78, 0x00, 0xfc, 0x00, 0xce, 0x01, 0x87, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xe3, 0xff, 0xde, 0xc0, 0xc0, 0xfe, 0x7e, // 'y' (0x79) : x=82, y=90, w=8
    0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x70, 0x38, 0x1c, 0x0e, 0xff, 0xff, 0x00, 0x00, 0x00, // 'z' (0x7a) : x=91, y=90, w=8
    0x1c, 0x1e, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x06, 0x06, 0x06, 0x06, 0x1e, 0x1c, 0x00, 0x00, // '{' (0x7b) : x=100, y=90, w=5
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, // '|' (0x7c) : x=106, y=90, w=2
    0x07, 0x0f, 0x0c, 0x0c, 0x0c, 0x0c, 0x1c, 0x1c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0f, 0x07, 0x00, 0x00, // '}' (0x7d) : x=109, y=90, w=5
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8e, 0xdf, 0xfb, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // '~' (0x7e) : x=115, y=90, w=8
};

static const CharInfo font16_index[] = {
    CharInfo(0, 3), CharInfo(16, 2), CharInfo(32, 7), CharInfo(48, 9), CharInfo(80, 8), CharInfo(96, 13), CharInfo(128, 10), CharInfo(160, 3), 
    CharInfo(176, 4), CharInfo(192, 4), CharInfo(208, 10), CharInfo(240, 8), CharInfo(256, 3), CharInfo(272, 7), CharInfo(288, 3), CharInfo(304, 8), 
    CharInfo(320, 8), CharInfo(336, 8), CharInfo(352, 8), CharInfo(368, 8), CharInfo(384, 8), CharInfo(400, 8), CharInfo(416, 8), CharInfo(432, 8), 
    CharInfo(448, 8), CharInfo(464, 8), CharInfo(480, 3), CharInfo(496, 3), CharInfo(512, 6), CharInfo(528, 6), CharInfo(544, 6), CharInfo(560, 8), 
    CharInfo(576, 10), CharInfo(608, 10), CharInfo(640, 10), CharInfo(672, 10), CharInfo(704, 10), CharInfo(736, 8), CharInfo(752, 8), CharInfo(768, 10), 
    CharInfo(800, 10), CharInfo(832, 2), CharInfo(848, 10), CharInfo(880, 10), CharInfo(912, 8), CharInfo(928, 12), CharInfo(960, 10), CharInfo(992, 10), 
    CharInfo(1024, 10), CharInfo(1056, 10), CharInfo(1088, 10), CharInfo(1120, 10), CharInfo(1152, 10), CharInfo(1184, 10), CharInfo(1216, 10), CharInfo(1248, 12), 
    CharInfo(1280, 10), CharInfo(1312, 10), CharInfo(1344, 10), CharInfo(1376, 4), CharInfo(1392, 8), CharInfo(1408, 4), CharInfo(1424, 7), CharInfo(1440, 7), 
    CharInfo(1456, 5), CharInfo(1472, 8), CharInfo(1488, 8), CharInfo(1504, 8), CharInfo(1520, 8), CharInfo(1536, 8), CharInfo(1552, 6), CharInfo(1568, 8), 
    CharInfo(1584, 8), CharInfo(1600, 3), CharInfo(1616, 4), CharInfo(1632, 7), CharInfo(1648, 3), CharInfo(1664, 10), CharInfo(1696, 8), CharInfo(1712, 8), 
    CharInfo(1728, 8), CharInfo(1744, 8), CharInfo(1760, 6), CharInfo(1776, 8), CharInfo(1792, 6), CharInfo(1808, 8), CharInfo(1824, 8), CharInfo(1840, 10), 
    CharInfo(1872, 11), CharInfo(1904, 8), CharInfo(1920, 8), CharInfo(1936, 5), CharInfo(1952, 2), CharInfo(1968, 5), CharInfo(1984, 8), 
};

Font font16(16, 32, 95, 2, font16_data, font16_index);

}
