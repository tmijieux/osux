#!/usr/bin/env python2
import fileinput

for l in fileinput.input():
    a = l.strip().split(" ")
    print ("%d, %d, %d,") % tuple(map(lambda x:int(x)-1, a))
