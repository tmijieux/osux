include $(top_srcdir)/common.mk

noinst_LTLIBRARIES = libosux_db.la
libosux_db_la_SOURCES = osuxdb.c

libosux_db_la_LIBADD = ../util/libosux_util.la

bin_PROGRAMS = makedb readdb
makedb_SOURCES = makedb.c
makedb_LDADD = libosux_db.la ../beatmap/libbeatmap.la

readdb_SOURCES = readdb.c
readdb_LDADD = libosux_db.la ../beatmap/libbeatmap.la

