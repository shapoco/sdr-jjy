#!/usr/bin/env python3

from PIL import Image

import sys
import numpy as np

import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-s', '--src-image', required=True)
parser.add_argument('-n', '--name', required=True)
parser.add_argument('-o', '--outdir', required=True)
parser.add_argument('-i', '--incdir', required=True)
parser.add_argument('-t', '--height', type=int, required=True)
parser.add_argument('-c', '--code-offset', type=int, required=True)
parser.add_argument('--spacing', type=int, default=-1)
args = parser.parse_args()

if args.spacing < 0:
    args.spacing = (args.height + 7) // 8

img = Image.open(args.src_image)
width, height = img.size
img_pixels = np.array([[img.getpixel((x,y)) for x in range(width)] for y in range(height)])

def is_red(pixel):
    return pixel[0] >= 128 and pixel[1] < 128 and pixel[2] < 128

class CharInfo:
    def __init__(self, c, x, y, w):
        self.code = c
        self.x = x
        self.y = y
        self.w = w
        self.index = 0

chars: list[CharInfo] = []

base_y = args.height
char_code = args.code_offset
while base_y < height:
    # ベースラインの赤い線を探す
    found = False
    for x in range(width):
        pixel = img_pixels[base_y, x]
        if is_red(pixel):
            found = True
            break
    if not found:
        base_y += 1
        continue
    
    # 文字毎に分解する
    last_is_red = False
    start_x = 0
    for x in range(width):
        pixel = img_pixels[base_y, x]
        curr_is_red = is_red(pixel)
        if not last_is_red and curr_is_red:
            start_x = x
        elif last_is_red and not curr_is_red:
            chars.append(CharInfo(char_code, start_x, base_y - args.height, x - start_x))
            char_code += 1
        elif curr_is_red and x == width - 1:
            chars.append(CharInfo(char_code, start_x, base_y - args.height, x + 1 - start_x))
            char_code += 1
        last_is_red = curr_is_red

    base_y += args.height + 1

data_array_name = f'{args.name}_data'
index_array_name = f'{args.name}_index'
namespace = 'bmpfont'

index = 0
with open(f'{args.outdir}/{args.name}.cpp', 'w') as f:
    f.write('#include <stdint.h>\n\n')
    f.write(f'#include "{args.incdir}/common.hpp"\n\n')
    f.write(f'namespace {namespace} {{\n\n')
    f.write(f'static const uint8_t {data_array_name}[] = {{\n')
    for ci in chars:
        ci.index = index
        sub_index = 0
        first_eol = True
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
                if (sub_index % 16 == 0):
                    f.write('    ')
                f.write('0x%02x, ' % byte)
                if ((sub_index + 1) % 16 == 0) or (x_step == stride -1 and y == args.height - 1):
                    if first_eol:
                        f.write("// '%s' (0x%02x) : x=%d, y=%d, w=%d" % (chr(ci.code), ci.code, ci.x, ci.y, ci.w))
                        first_eol = False
                    f.write('\n')
                sub_index += 1
                index += 1
    f.write('};\n\n')
    
    f.write(f'static const CharInfo {index_array_name}[] = {{\n')
    for ichar in range(len(chars)):
        ci = chars[ichar]
        if (ichar % 8 == 0):
            f.write('    ')
        f.write(f'CharInfo({ci.index}, {ci.w}), ')
        if ((ichar + 1) % 8 == 0) or ichar == len(chars) - 1:
            f.write('\n')
    f.write('};\n\n')
    
    f.write(f'Font {args.name}({args.height}, {args.code_offset}, {len(chars)}, {args.spacing}, {data_array_name}, {index_array_name});\n\n')
    
    f.write('}\n')

with open(f'{args.outdir}/{args.name}.hpp', 'w') as f:
    include_guard_symbol = f'{args.incdir}_{args.name}_HPP'.upper().replace('/', '_')
    f.write(f'#ifndef {include_guard_symbol}\n')
    f.write(f'#define {include_guard_symbol}\n')
    f.write('\n')
    f.write(f'#include "{args.incdir}/common.hpp"\n\n')
    f.write(f'namespace {namespace} {{\n\n')
    f.write(f'extern Font {args.name};\n\n')
    f.write('}\n\n')
    f.write('#endif\n')
