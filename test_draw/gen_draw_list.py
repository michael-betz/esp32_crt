#!/usr/bin/python3

from struct import pack
import json
import sys

T_NONE = 0
T_LINE = 1
T_POLY = 2
T_CIRCLE = 3
T_STRING = 4
T_STRING_LEFT = 4
T_STRING_CENTER = 5
T_STRING_RIGHT = 6
T_END = 0xFE

A_LEFT = 0
A_CENTER = 1
A_RIGHT = 2

fo = None


def line(density=100, x=0, y=0):
    fo.write(pack("BBhh", T_LINE, density, x, y))


def poly(density=100, pts=((0, 0), (100, 100))):
    fo.write(pack("BBH", T_POLY, density, len(pts)))
    for pt in pts:
        fo.write(pack("hh", pt[0], pt[1]))


def circle(density=100, x=0, y=0, r_x=100, r_y=100, a_start=0, a_length=0xFF):
    fo.write(pack(
        "BBhhhhBB", T_CIRCLE, density, x, y, r_x, r_y, a_start, a_length
    ))


def string(density=100, scale=100, x=0, y=0, s='Hello', align="left"):
    if align == "center":
        t = T_STRING_CENTER
    elif align == "right":
        t = T_STRING_RIGHT
    else:
        t = T_STRING_LEFT
    s_ = s.encode('ascii')
    fo.write(pack("BBHhhH", t, density, scale, x, y, len(s_)))
    fo.write(s_)


def end():
    fo.write(pack("B", T_END))


def main():
    global fo

    if len(sys.argv) != 2:
        print('Writes a binary draw-list to stdout. Usage:')
        print(sys.argv[0], 'input_json')
        print('\nDisplay draw list:')
        print(sys.argv[0], 'draw_list.json | ./test -')
        return

    fo = sys.stdout.buffer
    with open(sys.argv[1], 'r') as fi:
        dats = json.load(fi)

    for dat in dats:
        t = dat.pop('t')
        globals()[t](**dat)


if __name__ == '__main__':
    main()
