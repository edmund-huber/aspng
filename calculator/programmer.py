#!/usr/bin/env python3

import sys

import png

_, input_fn, bottom_left_x, bottom_left_y, target_r, target_g, target_b, expect_count, output_fn = sys.argv
bottom_left_x = int(bottom_left_x)
bottom_left_y = int(bottom_left_y)
target = (int(target_r), int(target_g), int(target_b))
expect_count = int(expect_count)

w, h, rows, details = png.Reader(file=open(input_fn, 'rb')).read()
out_rows = []
count = 0
for y, row in enumerate(rows):
    assert len(row) % 4 == 0
    out_row = []
    for x in range(len(row) // 4):
        rgba = tuple(row[x*4:(x*4)+4])
        if rgba[:3] == target and x >= bottom_left_x and y >= bottom_left_y:
            out_row.extend([255, 255, 255, 255])
            count += 1
        else:
            out_row.extend(rgba)
    out_rows.append(out_row)
assert count == expect_count

w = png.Writer(**details)
f = open(output_fn, 'wb')
w.write(f, out_rows)
