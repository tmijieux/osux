include $(top_srcdir)/common.mk

bin_PROGRAMS = dbctl

dbctl_SOURCES = dbctl.c cmdline.c cmdline.h dbctl.ggo
dbctl_CFLAGS = $(AM_CFLAGS) $(GLIB_CFLAGS)
dbctl_LDADD = ../../lib/libosux.la
