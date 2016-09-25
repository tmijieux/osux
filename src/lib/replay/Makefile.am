include $(top_srcdir)/common.mk
noinst_LTLIBRARIES = libosux_replay.la
libosux_replay_la_SOURCES = \
	replay.c \
	xz_decomp.c \
	hit.c \
	buffer_reader.c

libosux_replay_la_CFLAGS = $(AM_CFLAGS) $(LZMA_CFLAGS)
libosux_replay_la_LDFLAGS = $(AM_LDFLAGS) $(LZMA_LIBS)

