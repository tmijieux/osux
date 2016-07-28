include $(top_srcdir)/common.mk
noinst_LTLIBRARIES = libreplay.la
libreplay_la_SOURCES = \
	replay.c \
	xz_decomp.c

libreplay_la_CFLAGS = $(AM_CFLAGS)
libreplay_la_LDFLAGS = $(AM_LDFLAGS)
libreplay_la_LIBADD = ../util/libosux_util.la \
		      ../thirdparty/xz/src/liblzma/liblzma.la

bin_PROGRAMS = parse_replay

parse_replay_SOURCES = \
	main.c

parse_replay_LDADD = libreplay.la \
		../mod/libmod.la \
		../database/libosux_db.la \
		../beatmap/libbeatmap.la
