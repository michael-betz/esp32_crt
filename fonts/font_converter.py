'''
Convert font files (.ttf, .otf, .woff2) to the internally used font format (C-code file)
'''
import argparse
from pathlib import Path
from matplotlib.pyplot import plot, close, show, axis
from numpy import *

from fontTools import ttLib
from fontTools.pens.basePen import BasePen
from font_helpers import *


def convert(args):
    tt = ttLib.TTFont(args.font_file) # Load an existing font file
    tt.ensureDecompiled()
    cmap = tt["cmap"].getBestCmap()
    gs = tt.getGlyphSet()

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

        out_name = Path(args.font_file).with_suffix(".c")
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
        'font_file',
        help='Name of the .ttf, .otf or .woff2 file to convert'
    )
    args = parser.parse_args()

    convert(args)


if __name__ == '__main__':
    main()
