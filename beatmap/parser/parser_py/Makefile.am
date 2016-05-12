include $(top_srcdir)/common.mk

dist_python_DATA = osux_parse.py

pkglib_LTLIBRARIES = libosux_parser_py.la

libosux_parser_py_la_SOURCES = pyfetch.c pyfetch.h python.c python.h

libosux_parser_py_la_CFLAGS = $(PYTHON2_CFLAGS) $(AM_CFLAGS) \
				-DPYTHON_PATH=\"$(pythondir)\"

libosux_parser_py_la_LDFLAGS = -avoid-version -module -shared \
				-export-dynamic $(PYTHON2_LIBS)

