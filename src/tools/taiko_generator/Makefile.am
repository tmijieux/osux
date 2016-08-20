include $(top_srcdir)/common.mk

bin_PROGRAMS = taiko_generator

taiko_generator_SOURCES = taiko_generator.c cmdline.c cmdline.h taiko_generator.ggo
taiko_generator_CFLAGS = $(AM_CFLAGS) $(GLIB_CFLAGS)
taiko_generator_LDADD = ../../lib/libosux.la
