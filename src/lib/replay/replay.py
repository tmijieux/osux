#!/usr/bin/env python2

class Data:
    def __init__(self, line):
        offset, x, y, key = line.split("|")
        self.offset = int(offset)
        self.x = float(x)
        self.y = float(y)
        self.key = int(key)

    def __repr__(self):
        return "offset=%d, r_offset=%d, x=%g, y=%g, key=%d" \
        % (self.offset, self.r_offset, self.x, self.y, self.key)

    def __str__(self):
        return repr(self)

if __name__ == "__main__":
    import numpy as np
    import matplotlib.pyplot as plt
    
    print "kawabunga"

    ok = False
    prev = None
    X = []
    Y = []
    while 1:
        line = raw_input()
        if line == "Data:":
            ok = True
            continue
        if ok:
            d = Data(line)
            if d.offset == -12345:
                break
            if not prev is None:
                d.r_offset = prev.r_offset + d.offset
            else:
                d.r_offset = d.offset
            prev = d
            print d
            
            X.append(d.x)
            Y.append(d.y)
    
    plt.plot(X[2:], Y[2:])
    plt.show()
