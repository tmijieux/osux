include $(top_srcdir)/common.mk

lib_LTLIBRARIES = libosux_util.la 
libosux_util_la_SOURCES = \
	hash_table.c hash_table.h \
	heap.c heap.h \
	list.c list.h \
	split.c split.h \
	stack.c stack.h \
	yaml2.c yaml2.h \
	uleb128.c uleb128.h \
	re.c re.h \
	sum.c sum.h \
	data.c data.h \
	md5.c md5.h \
	string2.c string2.h

libosux_util_la_CFLAGS = $(CRYPTO_CFLAGS) $(AM_CFLAGS)
libosux_util_la_LDFLAGS = $(CRYPTO_LIBS) $(AM_LDFLAGS)
libosux_util_la_LIBADD = ../thirdparty/libyaml/src/libyaml.la

dist_python_DATA = osux_util.py
