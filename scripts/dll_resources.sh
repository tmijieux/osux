#!/bin/bash

if [ ! -f resources.tar.xz ]; then
    wget http://tmijieux.vvv.enseirb.fr/osux/resources.tar.xz
    #wget http://tmijieux.vvv.enseirb.fr/osux/bigdb.tar.xz
fi

tar xvf resources.tar.xz
if [ -d ./install ]; then
    cp resources/aamaps ./install/ -r
fi
