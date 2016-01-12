include $(top_srcdir)/common.mk
noinst_LTLIBRARIES = libreplay.la
libreplay_la_SOURCES = \
	replay.c \
	xz_decomp.c

libreplay_la_LIBADD = ../util/libutil.la
libreplay_la_LDFLAGS = -llzma

bin_PROGRAMS = parse_replay

parse_replay_SOURCES = \
	main.c

parse_replay_LDADD = ../mod/libmod.la libreplay.la
