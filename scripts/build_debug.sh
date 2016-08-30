#!/bin/bash

PREFIX=${PREFIX:-$HOME/prefix}
mkdir -p $PREFIX
rootdir=$(dirname $0)/../
$rootdir/configure --prefix=$PREFIX CFLAGS='-Wall -ggdb -DDEBUG=1 -O0 -fsanitize=address -fsanitize=undefined' LDFLAGS='-fsanitize=address -fsanitize=undefined'
