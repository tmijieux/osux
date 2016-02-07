#!/usr/bin/env python3

import sys

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("usage: {0} filename".format(sys.argv[0]))
        sys.exit(1)
    file_ = open(sys.argv[1], "rb")
    filename = sys.argv[1].split("/")[-1]
    fmt = "_".join(filename.split("."))
    print("const unsigned char _{}_data[] = {{{}}};".format(
        fmt, ",".join(str(b) for b in file_.read()) + ",0"))
    print("const unsigned long _{}_length = sizeof _{}_data;".format(
        fmt, fmt))
