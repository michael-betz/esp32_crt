'''
Convert simplified svg files to something understood by draw_blob().
Generates a .c file which can be included in the project
'''
import re
import argparse
import pathlib
from matplotlib.pyplot import plot, close, show, axis
from numpy import *
from svgpathtools import svg2paths, Path, Line, QuadraticBezier, CubicBezier, Arc

from font_helpers import *


class Drawer:
    def __init__(self, do_plot=False):
        self.do_plot = do_plot
        self.reset()

    def reset(self):
        self.last_point = (0, 0)
        self.bs = bytes()
        self.ce = CoordinateEncoder()
        self.cursor = [0, 0]
        self.lsb = 0

    def _lineTo(self, pt):
        if self.do_plot:
            pts = vstack((self.last_point, pt))
            pts += self.cursor
            pts[:, 0] += self.lsb
            plot(pts[:, 0], pts[:, 1], 'k.-')
            print('    lineTo', pt)
        self.bs += self.ce.encode(pt, T_LINETO << 4)
        self.last_point = pt

    def _moveTo(self, pt):
        if self.do_plot:
            print('    moveTo', pt)
        self.last_point = pt
        self.first_point = pt
        self.bs += self.ce.encode(pt, T_GOTO << 4)

    def _curveToOne(self, pt1, pt2, pt3):
        ''' draw a cubic bezier (never used for .ttf fonts) '''
        pt0 = self.last_point

        if self.do_plot:
            pts = vstack((pt0, pt1, pt2, pt3))
            pts += self.cursor
            pts[:, 0] += self.lsb
            plot(pts[:, 0], pts[:, 1], '.')

            xs, ys = get_points_c(pt0, pt1, pt2, pt3)
            plot(xs + self.cursor[0] + self.lsb, ys + self.cursor[1], 'k-')

            print('    cCurve', pt1, pt2, pt3)

        self.bs += self.ce.encode(pt1, T_CBEZ << 4)
        self.bs += self.ce.encode(pt2)
        self.bs += self.ce.encode(pt3)

        self.last_point = pt3

    def _qCurveToOne(self, pt1, pt2):
        ''' draw a quadratic bezier '''
        pt0 = self.last_point
        if self.do_plot:
            pts = vstack((pt0, pt1, pt2))
            pts += self.cursor
            pts[:, 0] += self.lsb
            plot(pts[:, 0], pts[:, 1], 'o')

            xs, ys = get_points(pt0, pt1, pt2)
            plot(xs + self.cursor[0] + self.lsb, ys + self.cursor[1], 'k-')

            print('    qCurve', pt1, pt2)

        self.bs += self.ce.encode(pt1, T_QBEZ << 4)
        self.bs += self.ce.encode(pt2)

        self.last_point = pt2

    def _closePath(self):
        if self.do_plot:
            print('    closePath')
        if self.last_point != self.first_point:
            self._lineTo(self.first_point)


def c2t(val):
    return round(real(val)), round(-imag(val))


def convert(args):
    paths, attributes = svg2paths(args.svg_file)
    print([a["id"] for a in attributes])

    d = Drawer(args.plot)

    for p in paths:
        for i, e in enumerate(p):
            pts = e.bpoints()

            if i == 0 or c2t(pts[0]) != d.last_point:
                d._moveTo(c2t(pts[0]))

            if isinstance(e, Line):
                d._lineTo(c2t(pts[1]))
            elif isinstance(e, QuadraticBezier):
                d._qCurveToOne(c2t(pts[1]), c2t(pts[2]))
            elif isinstance(e, CubicBezier):
                d._curveToOne(c2t(pts[1]), c2t(pts[2]), c2t(pts[3]))
            elif isinstance(e, Arc):
                print('Arcs are not implemented yet')

    # Add end marker
    d.bs += (T_END << 4).to_bytes(1, signed=False)

    if args.plot:
        axis("equal")
        show()

    combined_path = Path(*[seg for p in paths for seg in p._segments])
    bbox = combined_path.bbox()

    name = pathlib.Path(args.svg_file).name
    name = re.sub('[^A-Za-z0-9]+', '_', name)
    out_name = name + '.c'

    with open(out_name, 'w') as f:
        print(f'''\
#include <stdint.h>
// -----------------------------------
//  {name}
// xmin, xmax, ymin, ymax: {bbox[0]:.0f}, {bbox[1]:.0f}, {-bbox[3]:.0f}, {-bbox[2]:.0f},
// -----------------------------------

const uint8_t svg_{name}[{len(d.bs)}] = {{''', file=f)
        print_table(d.bs, f=f)

        print(f'wrote {out_name} ({len(d.bs) / 1024:.1f} kB)', )


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument(
        'svg_file',
        help='Name of the simplified .svg file to convert'
    )
    parser.add_argument('--plot', action='store_true', help='Plot some text to preview the font (Needs Matplotlib)')
    args = parser.parse_args()

    convert(args)


if __name__ == '__main__':
    main()
