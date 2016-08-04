#!/bin/bash

cd $(basename $0)
mkdir -p ../build/debug
cd ../build/debug
if [ ! -f Makefile ]; then
    rm -rf *
    cmake -DCMAKE_BUILD_TYPE=Debug -H../.. -B. \
          -DSANITIZE_ADDRESS=True -DSANITIZE_UNDEFINED=True
fi
make && make -s install 
