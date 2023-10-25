from struct import pack
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

f = None


def line(density, x_b, y_b):
    f.write(pack("BBhh", T_LINE, density, x_b, y_b))


# pts = [(-1, 0), (1, 0)]
def poly(density, pts):
    f.write(pack("BBH", T_POLY, density, len(pts)))
    for pt in pts:
        f.write(pack("hh", pt[0], pt[1]))


def circle(density, x, y, r_x, r_y, a_start=0, a_length=0xFF):
    f.write(pack(
        "BBhhhhBB", T_CIRCLE, density, x, y, r_x, r_y, a_start, a_length
    ))


def string(density, scale, x, y, s, align="left"):
    if align == "center":
        t = T_STRING_CENTER
    elif align == "right":
        t = T_STRING_RIGHT
    else:
        t = T_STRING_LEFT
    s_ = s.encode('ascii')
    f.write(pack("BBHhhH", t, density, scale, x, y, len(s_)))
    f.write(s_)


def end():
    f.write(pack("B", T_END))


def main():
    global f
    f = sys.stdout.buffer

    string(0x12, 0x34, 0x56, 0x78, "Hello!")
    end()


if __name__ == '__main__':
    main()
