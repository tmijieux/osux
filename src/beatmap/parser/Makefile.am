include $(top_srcdir)/common.mk

noinst_LTLIBRARIES =  libparser.la
libparser_la_SOURCES = parser.c
libparser_la_CFLAGS = $(AM_CFLAGS) -DPKG_LIB_DIR=\"$(pkglibdir)\"
libparser_la_LDFLAGS = $(AM_LDFLAGS) -ldl

dist_python_DATA = osux_parse.py

pkglib_LTLIBRARIES = libpyparser.la
libpyparser_la_SOURCES = pyfetch.c
libpyparser_la_LDFLAGS = -avoid-version -module -shared -export-dynamic
libpyparser_la_LIBADD = ../../python/libembedpython.la
libpyparser_la_DEPENDENCIES = ../../python/libembedpython.la
