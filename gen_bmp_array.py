#!/usr/bin/env python3

from PIL import Image

import sys
import numpy as np

import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-s', '--src', required=True)
parser.add_argument('--outcpp', required=True)
parser.add_argument('--outhpp', required=True)
parser.add_argument('-n', '--name', required=True)
args = parser.parse_args()

img = Image.open(args.src)
width, height = img.size
img_pixels = np.array([[img.getpixel((x,y)) for x in range(width)] for y in range(height)])

with open(args.outcpp, 'a') as f:
    f.write('\n')
    f.write(f'const uint8_t {args.name}[] = {{\n')
    f.write('    %d, %d, %d, %d,\n' % (width % 256, width // 256, height % 256, height // 256))
    index = 0
    stride = (width + 7) // 8
    length = stride * height
    for y in range(height):
        for x_step in range(stride):
            byte = 0
            for x_sub in range(8):
                x = x_step * 8 + x_sub
                if x < width:
                    pixel = img_pixels[y, x]
                    if pixel[0] != 0:
                        byte |= 1 << x_sub
            if (index % 16 == 0):
                f.write('    ')
            f.write('0x%02x, ' % byte)
            if (index % 16 == 15) or (index == length - 1):
                f.write('\n')
            index += 1
    f.write('};\n')

with open(args.outhpp, 'a') as f:
    f.write(f'extern const uint8_t {args.name}[];\n')
