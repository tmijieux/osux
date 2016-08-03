include $(top_srcdir)/common.mk

dist_python_DATA = osux_parse.py
pkglib_LTLIBRARIES = libosux_parser_py.la

libosux_parser_py_la_SOURCES = \
	pyfetch.c \
	python_internal.c python_internal.h 

libosux_parser_py_la_CFLAGS = \
	$(AM_CFLAGS) $(PYTHON2_CFLAGS) \
	-DMODULE_LOAD_PATH=\"$(pkglibdir)\" 

libosux_parser_py_la_LDFLAGS = \
	-avoid-version -module -shared -fPIC \
	-export-dynamic -no-undefined $(AM_LDFLAGS) $(PYTHON2_LIBS)
