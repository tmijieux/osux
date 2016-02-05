include $(top_srcdir)/common.mk

noinst_LTLIBRARIES = libembedpython.la
libembedpython_la_SOURCES = python.c python.h
libembedpython_la_CFLAGS = $(PYTHON2_CFLAGS) $(AM_CFLAGS) \
			-DPYTHON_PATH=\"$(pythondir)\"
libembedpython_la_LDFLAGS = $(PYTHON2_LIBS) $(AM_LDFLAGS)

