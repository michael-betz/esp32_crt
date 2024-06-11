'''
Convert true-type files to the internally used font format (C-code file)
'''
import argparse
from pathlib import Path
from matplotlib.pyplot import plot, close, show, axis
from numpy import *

from fontTools import ttLib
from fontTools.pens.basePen import BasePen
from font_helpers import *

DENSITY = 0.1

def get_points(pt0, pt1, pt2):
    dx = pt2[0] - pt0[0]
    dy = pt2[1] - pt0[1]
    dist = sqrt(dx**2 + dy**2)
    N_POINTS = round(dist * DENSITY)
    if N_POINTS < 3:
        xs = array([pt0[0], (pt0[0] + pt2[0]) / 4 + pt1[0] / 2, pt2[0]])
        ys = array([pt0[1], (pt0[1] + pt2[1]) / 4 + pt1[1] / 2, pt2[1]])
        return xs, ys

    t = linspace(0, 1, N_POINTS)
    xs = (1 - t)**2 * pt0[0] + 2 * (1 - t) * t * pt1[0] + t**2 * pt2[0]
    ys = (1 - t)**2 * pt0[1] + 2 * (1 - t) * t * pt1[1] + t**2 * pt2[1]
    return xs, ys


class CrtPen(BasePen):
    def reset(self):
        self.last_point = (0, 0)
        self.bs = bytes()
        self.ce = CoordinateEncoder()
        self.cursor = [0, 0]
        self.lsb = 0

    def __init__(self, *args, do_plot=False, **kwargs):
        self.do_plot = do_plot
        self.reset()
        super(CrtPen, self).__init__(*args, **kwargs)

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

    def _qCurveToOne(self, pt1, pt2):
        pt0 = self.last_point
        if self.do_plot:
            pts = vstack((pt0, pt1, pt2))
            pts += self.cursor
            pts[:, 0] += self.lsb
            plot(pts[:, 0], pts[:, 1], 'o')

            xs, ys = get_points(pt0, pt1, pt2)
            plot(xs + self.cursor[0] + self.lsb, ys + self.cursor[1], 'k.-')

            print('    qCurve', pt1, pt2)

        self.bs += self.ce.encode(pt1, T_QBEZ << 4)
        self.bs += self.ce.encode(pt2)

        self.last_point = pt2

    def _closePath(self):
        if self.do_plot:
            print('    closePath')
        if self.last_point != self.first_point:
            self._lineTo(self.first_point)

    def _endPath(self):
        print('endPath')


def convert(args):
    tt = ttLib.TTFont(args.ttf_file) # Load an existing font file
    tt.ensureDecompiled()
    cmap = tt["cmap"].getBestCmap()
    gs = tt.getGlyphSet()
    glyf = tt['glyf']

    crt_pen = CrtPen(gs, do_plot=args.plot)

    if args.plot:
        # Make a plot
        for c in args.text:
            name = cmap[ord(c)]
            g = gs[name]
            print(f'"{c}": {name:16s} {g.lsb:4d} {g.width:4d}')
            g.draw(crt_pen)
            crt_pen.cursor[0] += g.width
            axis("equal")
            show()

    else:
        # Generate code
        all_bs = bytes()
        glyph_props = []

        for c in arange(0x20, 0x20 + 95):
            name = cmap[c]
            g = gs[name]

            crt_pen.reset()
            g.draw(crt_pen)

            glyph_props.append({
        #         "start_index": len(all_bs),
                "end_index": len(all_bs) + len(crt_pen.bs),
                "adv_w": g.width,
        #         "ofs_x": g.lsb
            })

            all_bs += crt_pen.bs

        # print(len(all_bs))
        # print(glyph_props)

        out_name = Path(args.ttf_file).with_suffix(".c")
        name = tt["name"].getBestFullName().lower()

        with open(out_name, 'w') as f:
            print(f'''\
// -----------------------------------
//  {tt["name"].getBestFullName()} (.ttf font)
// -----------------------------------

static const uint8_t glyphs_{name}[{len(all_bs)}] = {{''', file=f)
            print_table(all_bs, f=f)

            print(f'''\
// GLYPH DESCRIPTION
static const glyph_dsc_t glyph_dsc_{name}[{len(glyph_props)}] = {{''', file=f)
            for i, line in enumerate(glyph_props):
                print("    {" + ", ".join([f'.{k} = {v:5d}' for k, v in line.items()]) + f"}},  // '{chr(0x20 + i)}'", file=f)
            print("};", file=f)

            print(f'''
const font_t f_{name} = {{
    .units_per_em = {tt['head'].unitsPerEm},
    .n_glyphs = {len(glyph_props)},
    .glyphs = glyphs_{name},
    .glyph_dsc = glyph_dsc_{name},
}};
            ''', file=f)

        print(f'wrote {out_name} ({len(all_bs) / 1024:.1f} kB)', )


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('--plot', action='store_true', help='Plot some text to preview the font (Needs Matplotlib)')
    parser.add_argument('--text', default="(123)", help='String to plot for preview')
    parser.add_argument(
        'ttf_file',
        help='Name of the .ttf file to convert'
    )
    args = parser.parse_args()

    convert(args)


if __name__ == '__main__':
    main()
