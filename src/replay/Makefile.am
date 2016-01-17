include $(top_srcdir)/common.mk
noinst_LTLIBRARIES = libreplay.la
libreplay_la_SOURCES = \
	replay.c \
	xz_decomp.c

libreplay_la_LIBADD = ../util/libosux_util.la
libreplay_la_CFLAGS = $(LZMA_CFLAGS) $(AM_CFLAGS)
libreplay_la_LDFLAGS = $(LZMA_LIBS) $(AM_LDFLAGS)

bin_PROGRAMS = parse_replay

parse_replay_SOURCES = \
	main.c

parse_replay_LDADD = libreplay.la ../mod/libmod.la
