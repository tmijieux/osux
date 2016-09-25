include $(top_srcdir)/common.mk

noinst_LTLIBRARIES = libosux_util.la 
libosux_util_la_SOURCES = \
	hash_table.c \
	heap.c \
	list.c \
	stack.c \
	yaml.c \
	sum.c \
	md5.c \
	string.c \
	error.c

libosux_util_la_CFLAGS = $(CRYPTO_CFLAGS) $(YAML_CFLAGS) $(AM_CFLAGS)
libosux_util_la_LDFLAGS = $(CRYPTO_LIBS) $(YAML_LIBS) $(AM_LDFLAGS)
