#!/usr/bin/python3

import sys
import yaml
import matplotlib.pylab as mp

def plot(x, y, name):
    mp.plot(x, y, linewidth = 1.0)
    mp.xlabel('')
    mp.ylabel('')
    mp.title(name)
    mp.show()

def lf_plot(data, name):
    l = data[name + "_length"]
    lx = []
    ly = []
    for i in range(1, l+1):
        x = float(data[name + "_x" + str(i)])
        y = float(data[name + "_y" + str(i)])
        if i == l and ly[l-2] == y:
            # avoid some ugly plot with nothing visible
            continue
        lx.append(x)
        ly.append(y)
    plot(lx, ly, name)

def usage():
    print("""Usage:
    ./tr_linear_fun.py path/to/file.yaml vect_name1 vect_name2 ...
    """)

if __name__ == '__main__':
    if len(sys.argv) < 3:
        usage()
        sys.exit(0)
    filepath = sys.argv[1]
    with open(filepath) as f:
        data = yaml.load(f)
        for name in sys.argv[2:]:
            lf_plot(data, name)
