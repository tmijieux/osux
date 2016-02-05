include $(top_srcdir)/common.mk

lib_LTLIBRARIES = libosux_util.la 
libosux_util_la_SOURCES = \
	hash_table.c \
	heap.c \
	list.c \
	split.c \
	stack.c \
	yaml2.c \
	uleb128.c \
	re.c \
	sum.c \
	data.c \
	md5.c

libosux_util_la_CFLAGS = $(YAML_CFLAGS) $(AM_CFLAGS) $(CRYPTO_CFLAGS)
libosux_util_la_LDFLAGS = $(YAML_LIBS) $(AM_LDFLAGS) $(CRYPTO_LIBS)

dist_python_DATA = osux_util.py
