include $(top_srcdir)/common.mk

SUBDIRS = dbctl

bin_PROGRAMS = parse_replay taiko_converter parse_beatmap

parse_replay_SOURCES = parse_replay.c
parse_replay_LDADD = ../lib/libosux.la

taiko_converter_SOURCES = taiko_converter.c
taiko_converter_LDADD = ../lib/libosux.la

parse_beatmap_SOURCES = beatmap_parser.c
parse_beatmap_LDADD = ../lib/libosux.la
