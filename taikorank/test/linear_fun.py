#!/usr/bin/python3

# Copyright (©) 2015-2016 Lucas Maugère, Thomas Mijieux
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#    http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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
