#!/bin/bash

pacman -Syu mingw-w64-x86_64-toolchain mingw-w64-x86_64-libtool \
	mingw-w64-x86_64-xz mingw-w64-x86_64-libyaml \
	mingw-w64-x86_64-sqlite3 mingw-w64-x86_64-glib2 \
	mingw-w64-x86_64-python2 mingw-w64-x86_64-openssl \
	mingw-w64-x86_64-libmariadbclient
