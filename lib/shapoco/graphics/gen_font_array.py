#!/usr/bin/env python3

from PIL import Image

import sys
import numpy as np

import argparse

LIB_NAMESPCAE = '::shapoco::graphics'

parser = argparse.ArgumentParser()
parser.add_argument('-s', '--src', required=True)
parser.add_argument('-n', '--name', required=True)
parser.add_argument('-o', '--outdir', required=True)
parser.add_argument('-i', '--incdir', default='shapoco/graphics')
parser.add_argument('-t', '--height', type=int, default=-1)
parser.add_argument('-c', '--code-offset', type=int, required=True)
parser.add_argument('--cpp_namespace', default=LIB_NAMESPCAE)
parser.add_argument('--spacing', type=int, default=-1)
args = parser.parse_args()

img = Image.open(args.src)
width, height = img.size
img_pixels = np.array([[img.getpixel((x,y)) for x in range(width)] for y in range(height)])

def is_red(pixel):
    return pixel[0] >= 128 and pixel[1] < 128 and pixel[2] < 128

def is_blue(pixel):
    return pixel[0] < 128 and pixel[1] < 128 and pixel[2] >= 128

def is_valid(pixel):
    return is_red(pixel) or is_blue(pixel)

class TinyFontGlyph:
    def __init__(self, c, x, y, w):
        self.code = c
        self.x = x
        self.y = y
        self.w = w
        self.index = 0
        self.valid = False
        self.blank = True

chars: list[TinyFontGlyph] = []

base_y = 1
first_valid_code = args.code_offset
last_valid_code = None
num_valid_chars = 0
char_code = first_valid_code
while base_y < height:
    # ベースラインの赤い線を探す
    found = False
    for x in range(width):
        pixel = img_pixels[base_y, x]
        if is_valid(pixel):
            found = True
            break
    if not found:
        base_y += 1
        continue
    
    # 文字高を自動決定
    if args.height <= 0:
        args.height = base_y
        print(f"*INFO: Font height for '{args.name}' was automatically determined: {args.height}px")
    
    # 文字毎に分解する
    last_is_valid = False
    first_is_blue = False
    start_x = 0
    for x in range(width):
        pixel = img_pixels[base_y, x]
        curr_is_valid = is_valid(pixel)
        if not last_is_valid and curr_is_valid:
            start_x = x
            first_is_blue = is_blue(pixel)
        else:
            end_of_underline = last_is_valid and not curr_is_valid
            end_of_line = curr_is_valid and x == width - 1
            if end_of_underline or end_of_line:
                if first_is_blue:
                    char_w = 0
                elif end_of_underline:
                    char_w = (x - start_x)
                else:
                    char_w = (x + 1 - start_x)

                ci = TinyFontGlyph(char_code, start_x, base_y - args.height, char_w)
                chars.append(ci)
                ci.valid = char_w != 0
                
                if ci.valid:
                    last_valid_code = char_code
                    num_valid_chars += 1
                elif char_code == first_valid_code:
                    first_valid_code += 1
                
                char_code += 1
        last_is_valid = curr_is_valid

    base_y += args.height + 1

valid_code_range = last_valid_code + 1 - first_valid_code

if first_valid_code != args.code_offset:
    print(f"*INFO: code offset changed: {args.code_offset} --> {first_valid_code}.")

if num_valid_chars > 0:
    print(f"*INFO: {num_valid_chars} valid chars found.")
else:
    raise(Exception('No valid char found.'))

if args.spacing < 0:
    args.spacing = (args.height + 7) // 8
    print(f"*INFO: Char spacing for '{args.name}' was automatically determined: {args.spacing}px")

chars = chars[first_valid_code - args.code_offset:last_valid_code + 1 - args.code_offset]

data_array_name = f'{args.name}_data'
index_array_name = f'{args.name}_index'

index = 0
with open(f'{args.outdir}/{args.name}.cpp', 'w') as f:
    f.write('#include <stdint.h>\n\n')
    f.write(f'#include "{args.incdir}/tiny_font.hpp"\n\n')
    f.write(f'namespace {args.cpp_namespace} {{\n\n')
    if args.cpp_namespace != LIB_NAMESPCAE:
        f.write(f'using namespace {LIB_NAMESPCAE};\n\n')
    f.write(f'static const uint8_t {data_array_name}[] = {{\n')
    for ci in chars:
        ci.index = index
        ci.blank = True
        bytes = []
        for y in range(args.height):
            stride = (ci.w + 7) // 8
            for x_step in range(stride):
                byte = 0
                for x_sub in range(8):
                    x = x_step * 8 + x_sub
                    if x < ci.w:
                        pixel = img_pixels[ci.y + y, ci.x + x]
                        if pixel[0] != 0:
                            byte |= 1 << x_sub
                bytes.append(byte)
                if byte != 0:
                    ci.blank = False
        
        comment = "'%s' (0x%02x) : index=%d, w=%d" % (chr(ci.code), ci.code, index, ci.w)
        if ci.blank:
            f.write(f'    // (BLANK) {comment}\n')
        elif ci.w == 0:
            f.write(f'    // (SKIP) {comment}\n')
        else:
            first_eol = True
            sub_index = 0
            for byte in bytes:
                if (sub_index % 16 == 0):
                    f.write('    ')
                f.write('0x%02x, ' % byte)
                if ((sub_index + 1) % 16 == 0) or (sub_index + 1 == len(bytes)):
                    if first_eol:
                        f.write(f'// {comment}')
                        first_eol = False
                    f.write('\n')
                sub_index += 1
                index += 1

    f.write('};\n\n')
    
    CHAR_INFO_COLS = 1
    f.write(f'static const TinyFontGlyph {index_array_name}[] = {{\n')
    for ichar in range(len(chars)):
        ci = chars[ichar]
        optional_args = ''

        flags = []
        if ci.blank:
            flags.append('TinyFontGlyph::BLANK')
        if len(flags) > 0:
            optional_args += f', {' | '.join(flags)}'
        
        if (ichar % CHAR_INFO_COLS == 0):
            f.write('    ')
        f.write(f'TinyFontGlyph({ci.index}, {ci.w}{optional_args}), ')
        if ((ichar + 1) % CHAR_INFO_COLS == 0) or ichar == len(chars) - 1:
            f.write('\n')
    f.write('};\n\n')
    
    f.write(f'TinyFont {args.name}({args.height}, {first_valid_code}, {len(chars)}, {args.spacing}, {data_array_name}, {index_array_name});\n\n')
    
    f.write('}\n')

with open(f'{args.outdir}/{args.name}.hpp', 'w') as f:
    f.write(f'#pragma once\n')
    f.write('\n')
    f.write(f'#include "{args.incdir}/tiny_font.hpp"\n\n')
    f.write(f'namespace {args.cpp_namespace} {{\n\n')
    f.write(f'extern {LIB_NAMESPCAE}::TinyFont {args.name};\n\n')
    f.write('}\n')
