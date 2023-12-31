#!/usr/bin/python3
'''\
Converts a draw-list from a .json file into the binary
format of esp_crt. Outputs the binary data on stdout

Usage:
./json_to_bin.py draw_list.json | hexdump -C

Or pipe the result directly to the CRT-simulator:
./json_to_bin.py draw_list.json | ./test -
'''
from struct import pack
import json
import sys

T_NONE = 0
T_LINE = 1
T_POLY = 2
T_CIRCLE = 3
T_STRING = 4
T_END = 0xFE

A_LEFT = 0
A_CENTER = 1
A_RIGHT = 2

fo = None


def line(density=50, x=0, y=0):
    fo.write(pack("BBhh", T_LINE, density, x, y))


def poly(density=50, pts=((0, 0), (100, 100))):
    fo.write(pack("BBH", T_POLY, density, len(pts)))
    for pt in pts:
        fo.write(pack("hh", pt[0], pt[1]))


def circle(density=50, x=0, y=0, r_x=100, r_y=0, a_start=0, a_length=0xFF):
    fo.write(pack(
        "BBhhhhBB", T_CIRCLE, density, x, y, r_x, r_y, a_start, a_length
    ))


def string(density=50, scale=100, x=0, y=0, font=0, s='Hello', align="center"):
    t = T_STRING
    if align == "center":
        t |= A_CENTER
    elif align == "right":
        t |= A_RIGHT
    else:
        t |= A_LEFT
    s_ = s.encode('ascii')
    fo.write(pack("BBHhhBB", t, density, scale, x, y, font, len(s_)))
    fo.write(s_)


def end():
    fo.write(pack("B", T_END))


def main():
    global fo

    if len(sys.argv) != 2:
        print(__doc__, file=sys.stderr)
        return

    fo = sys.stdout.buffer
    with open(sys.argv[1], 'r') as fi:
        ls = fi.readlines()

    # remove comments
    ls = [l for l in ls if not l.strip().startswith('//')]
    dats = json.loads(''.join(ls))
    print(f'read {len(dats)} items from {sys.argv[1]}', file=sys.stderr)

    for dat in dats:
        t = dat.pop('t')
        globals()[t](**dat)


if __name__ == '__main__':
    main()
