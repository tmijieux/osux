#!/bin/bash

INSTALLDIR="/c/osux"
BINDIR=$INSTALLDIR/bin
DATADIR=$INSTALLDIR/share
CONFDIF=$INSTALLDIR/etc
LOCALEDIR=$DATADIR/locale

[[ $# -ne 1 ]] && exit 1
ldd $1 | grep -E '/msys64.*dll' | cut -d'<' -f2 | cut -d' ' -f2 | xargs -i cp {} $BINDIR 

mkdir -p $LOCALEDIR
find . -name '*.mo' | grep -E 'glib|gtk|pango|gdk' | xargs -i cp -r {} /msys64/share/locale/

mkdir -p $DATADIR/glib-2.0/
cp -r /msys64/share/glib-2.0/schemas $DATADIR/glib-2.0/
cp -r /msys64/share/icons $DATADIR 

