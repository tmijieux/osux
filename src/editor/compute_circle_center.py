#!/usr/bin/env python2

import numpy as np;
import matplotlib.pyplot as plt

if __name__ == "__main__":
    print "kawabunga"
    x1=296; y1=368
    x2=232; y2=380
    x3=140; y3=384

    Xa = 0.5*(x1+x2); Ya = 0.5*(y1+y2);
    Xb = 0.5*(x2+x3); Yb = 0.5*(y2+y3);
    print "Xa=%g; Ya=%g; Xb = %g; Yb =%g" % ( Xa, Ya, Xb, Yb);

    A1 = y2-y1; B1 = x2-x1;
    A2 = y3-y2; B2 = x3-x2;
    print "A1=%g; A2=%g; B1 = %g; B2 =%g" % ( A1, A2, B1, B2);
    
    det = B1*A2-B2*A1;
    if (np.fabs(det) < 0.00001):
        exit

    print "det=%g" % det
    p = (A2*B1*Xa - (B2*Xb - A2*Ya + A2*Yb)*A1)/det
    q = -(A1*B2*Ya + (B2*Xa - B2*Xb - A2*Yb)*B1)/det
    print "p=%g; q=%g" % (p, q)
    
