import numpy as np
from matplotlib.pyplot import plot
from fontTools.pens.basePen import BasePen


# TYPES
T_GOTO = 0
T_LINETO = 1
T_QBEZ = 2
T_CBEZ = 3
T_ARC = 4
T_END = 0xF

# for preview render only
DENSITY = 0.1


class CoordinateEncoder:
    def __init__(self, N=2):
        '''
        Encodes a int16 x, y coordinate pair (N=2) into a blob of 1 - 5 bytes
        The upper 4 bits of the first byte are not used. The lower 4 bits are the flags.
        flags for N = 2: [positive_1, positive_0, short_1, short_0]
        '''
        self.prev = [0] * N
        self.N = N
        self.min_max = [MinMax() for _ in range(N)]

    def encode(self, vals, flag_or=0):
        flags = 0
        return_bytes = bytes()

        for i in range(self.N):  # For each dimension (X and Y for N = 2)
            val = round(vals[i])
            self.min_max[i].add(val)
            d_val = val - self.prev[i]

            if d_val == 0:
                # Repeat previous value. Make sure the bits F_X_SHORT and F_X_POS are cleared
                pass
            elif abs(d_val) <= 255:
                # Use short number format with relative encoding as uint8. Set the F_X_SHORT bit.
                flags |= (1 << i)
                return_bytes += abs(d_val).to_bytes(1, signed=False)
                # set the F_X_POS bit if the number is positive
                if d_val > 0:
                    flags |= (1 << (i + self.N))
            else:
                # Use long number format with absolute encoding as int16. Set the F_X_POS bit.
                flags |= (1 << (i + self.N))
                return_bytes += val.to_bytes(2, signed=True)

            self.prev[i] = val

        flags |= flag_or
        return flags.to_bytes(1, signed=False) + return_bytes


class MinMax:
    def __init__(self):
        self.reset()

    def add(self, val):
        if self.min_val is None or val < self.min_val:
            self.min_val = val

        if self.max_val is None or val > self.max_val:
            self.max_val = val

    def get_min(self):
        return 0 if self.min_val is None else self.min_val

    def get_max(self):
        return 0 if self.max_val is None else self.max_val

    def reset(self):
        self.min_val = None
        self.max_val = None


def print_table(vals, w=24, w_v=3, f=None):
    ll = len(vals) - 1
    for i, g in enumerate(vals):
        if i > 0 and (i % w) == 0:
            print(file=f)
        print(f'{g:{w_v}d}', end='', file=f)
        if i < ll:
            print(',', end='', file=f)
    print('\n};\n', file=f)


def get_points(pt0, pt1, pt2):
    ''' render a quadratic bezier for preview in plot window '''
    dx = pt2[0] - pt0[0]
    dy = pt2[1] - pt0[1]
    dist = np.sqrt(dx**2 + dy**2)
    N_POINTS = np.round(dist * DENSITY)
    if N_POINTS < 3:
        xs = np.array([pt0[0], (pt0[0] + pt2[0]) / 4 + pt1[0] / 2, pt2[0]])
        ys = np.array([pt0[1], (pt0[1] + pt2[1]) / 4 + pt1[1] / 2, pt2[1]])
        return xs, ys

    t = np.linspace(0, 1, N_POINTS)
    xs = (1 - t)**2 * pt0[0] + 2 * (1 - t) * t * pt1[0] + t**2 * pt2[0]
    ys = (1 - t)**2 * pt0[1] + 2 * (1 - t) * t * pt1[1] + t**2 * pt2[1]
    return xs, ys


def get_points_c(pt0, pt1, pt2, pt3):
    ''' render a cubic bezier for preview in plot window '''
    dx = pt2[0] - pt0[0]
    dy = pt2[1] - pt0[1]
    dist = np.sqrt(dx**2 + dy**2)
    N_POINTS = round(dist * DENSITY)
    if N_POINTS < 3:
        xs = np.array([pt0[0], (pt0[0] + 3 * (pt1[0] + pt2[0]) + pt3[0]) / 8, pt3[0]])
        ys = np.array([pt0[1], (pt0[1] + 3 * (pt1[1] + pt2[1]) + pt3[1]) / 8, pt3[1]])
        return xs, ys

    t = np.linspace(0, 1, N_POINTS)

    xs = (1 - t)**3 * pt0[0] + 3 * (1 - t)**2 * t * pt1[0] + 3 * (1 - t) * t**2 * pt2[0] + t**3 * pt3[0]
    ys = (1 - t)**3 * pt0[1] + 3 * (1 - t)**2 * t * pt1[1] + 3 * (1 - t) * t**2 * pt2[1] + t**3 * pt3[1]
    return xs, ys


class CrtPen(BasePen):
    def __init__(self, *args, do_plot=False, **kwargs):
        ''' receive and convert coordinate and shape data from fontTools '''
        self.do_plot = do_plot
        self.reset()
        super(CrtPen, self).__init__(*args, **kwargs)

    def reset(self):
        self.last_point = (0, 0)
        self.bs = bytes()
        self.ce = CoordinateEncoder()
        self.cursor = [0, 0]
        self.lsb = 0

    def _lineTo(self, pt):
        if self.do_plot:
            pts = np.vstack((self.last_point, pt))
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
            pts = np.vstack((pt0, pt1, pt2, pt3))
            pts += self.cursor
            pts[:, 0] += self.lsb
            plot(pts[:, 0], pts[:, 1], 'o')

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
            pts = np.vstack((pt0, pt1, pt2))
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

    def _endPath(self):
        print('endPath')
