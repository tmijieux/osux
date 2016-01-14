include $(top_srcdir)/common.mk

noinst_LTLIBRARIES = libpyparser.la libparser.la
libparser_la_SOURCES = parser.c
libpyparser_la_SOURCES = pyfetch.c

libparser_la_LIBADD = libpyparser.la

dist_python_DATA = osux_parse.py
