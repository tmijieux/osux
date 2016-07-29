include $(top_srcdir)/common.mk

SUBDIRS = parser_py parser_c

noinst_LTLIBRARIES =  libosux_parser.la
libosux_parser_la_SOURCES = parser.c parser.h

libosux_parser_la_CFLAGS = \
	$(AM_CFLAGS) \
	-DMODULE_LOAD_PATH=\"$(pkglibdir)\"

libosux_parser_la_LDFLAGS = $(AM_LDFLAGS)

