'''
Convert font files (.ttf, .otf, .woff2) to the internally used font format (C-code file)
'''
import re
import argparse
from pathlib import Path
from matplotlib.pyplot import plot, close, show, axis
from numpy import *

from fontTools import ttLib
from fontTools.pens.basePen import BasePen
from font_helpers import *


def get_cp_set(args, cmap):
    ''' assemble the set of code-points to convert '''
    cp_set = set()

    if args.add_range is not None:
        cp_set.update((int(x, base=0) for x in args.add_range.split(",")))

    if args.add_ascii:
        cp_set.update(range(0x20, 0x20 + 95))

    if args.add_all:
        cp_set.update(cmap.keys())

    # We don't support ascii codes below 0x20
    cp_set.difference_update(range(0x20))

    return sorted(cp_set)


def get_n_ascii(cp_set):
    ''' how many characters correspond to the basic ascii scheme? '''
    first_char = None
    for i, cp in enumerate(cp_set):
        if i == 0:
            first_char = cp
        elif first_char + i != cp:
            return(first_char, i)
    return(first_char, len(cp_set))


def convert(args):
    tt = ttLib.TTFont(args.font_file) # Load an existing font file
    tt.ensureDecompiled()

    # Map between unicode code-point and glyph name (key for the GlyphSet)
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

        cp_set = get_cp_set(args, cmap)
        firstchar, n_ascii = get_n_ascii(cp_set)

        for c in cp_set:
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

        name = tt["name"].getBestFullName().lower()
        name = re.sub('[^A-Za-z0-9]+', '_', name)
        out_name = Path(name).with_suffix(".c")

        with open(out_name, 'w') as f:
            print(f'''\
#include <stdint.h>
#include <stdio.h>
#include <font_draw.h>
// -----------------------------------
//  {tt["name"].getBestFullName()}
// -----------------------------------

static const uint8_t glyphs_{name}[{len(all_bs)}] = {{''', file=f)
            print_table(all_bs, f=f)

            print(f'''\
// GLYPH DESCRIPTION
static const glyph_dsc_t glyph_dsc_{name}[{len(glyph_props)}] = {{''', file=f)
            for cp, line in zip(cp_set, glyph_props):
                print("    {" + ", ".join([f'.{k} = {v:5d}' for k, v in line.items()]) + f"}},  // U+{cp:04X} '{chr(cp)}' {cmap[cp]}", file=f)
            print("};\n", file=f)

            n_cp_table_entries = len(cp_set) - n_ascii
            cp_table_name = "NULL"
            if n_cp_table_entries > 0:
                cp_table_name = f"code_points_{name}"
                print(f'static const unsigned {cp_table_name}[{n_cp_table_entries}] = {{', file=f)
                print_table(cp_set[n_ascii:], w=19, w_v=6, f=f)

            print(f'''\
const font_t f_{name} = {{
    .units_per_em = {tt['head'].unitsPerEm},
    .n_glyphs = {len(glyph_props)},
    .map_start = {firstchar},
    .map_n = {n_ascii},
    .map_unicode_table = {cp_table_name},
    .glyphs = glyphs_{name},
    .glyph_dsc = glyph_dsc_{name},
}};
            ''', file=f)

        print(f'wrote {out_name} ({len(all_bs) / 1024:.1f} kB)', )


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('--plot', action='store_true', help='Plot some text to preview the font (Needs Matplotlib)')
    parser.add_argument('--text', default="(123)", help='String to plot for preview')
    parser.add_argument('--add-ascii', action='store_true', help='Add 95 Ascii characters to the generated file')
    parser.add_argument('--add-all', action='store_true', help='Add all glyphs in the font')
    parser.add_argument('--add-range', help='Add comma separated list of unicodes to the generated file')

    parser.add_argument(
        'font_file',
        help='Name of the .ttf, .otf or .woff2 file to convert'
    )
    args = parser.parse_args()

    convert(args)


if __name__ == '__main__':
    main()
