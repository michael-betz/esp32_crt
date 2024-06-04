# TYPES
T_GOTO = 0
T_LINETO = 1
T_QBEZ = 2
T_ARC = 4
T_END = 0xF

class CoordinateEncoder:
    def __init__(self, N=2):
        '''
        Encodes a int16 x, y coordinate pair (N=2) into a blob of 1 - 5 bytes
        The upper 4 bits of the first byte are not used. The lower 4 bits are the flags.
        flags for N = 2: [positive_1, positive_0, short_1, short_0]
        '''
        self.prev = [0] * N
        self.N = N

    def encode(self, vals, flag_or=0):
        flags = 0
        return_bytes = bytes()

        for i in range(self.N):  # For each dimension (X and Y for N = 2)
            val = round(vals[i])
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
