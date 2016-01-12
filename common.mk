AM_CFLAGS = -fpic -Wall -Wextra -I$(top_srcdir)/src -I$(top_srcdir)/src/util -I$(top_srcdir)/src/include
AM_LDFLAGS=-Wl,-rpath,LIBDIR
