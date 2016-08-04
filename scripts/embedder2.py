#!/usr/bin/env python2

import sys
import binascii

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "usage: %s filename" % sys.argv[0]
        sys.exit(1)
    file_ = open(sys.argv[1], "rb")
    filename = sys.argv[1].split("/")[-1]
    fmt = ".".join(filename.split(".")[:-1])

    h = open(filename+".h", "w+")
    c = open(filename+".c", "w+")
    
    h.write("extern const unsigned char _%s_data[];\n" % fmt)
    h.write("extern const unsigned long _%s_length;\n" % fmt)

    c.write("#include \"./%s\"\n" % (filename+".h"))
    c.write("const unsigned char _%s_data[] = {%s};\n" %
            (fmt, ",".join("0x"+binascii.hexlify(b) for b in file_.read()) + ",0"))
    c.write("const unsigned long _%s_length = sizeof _%s_data;" % (fmt,fmt))
