include $(top_srcdir)/common.mk

BUILT_SOURCES = _db_sql_data.c
noinst_LTLIBRARIES = libosux_db.la
libosux_db_la_SOURCES = osux_db.c _db_sql_data.c db.sql

libosux_db_la_LIBADD = ../util/libosux_util.la

bin_PROGRAMS = makedb readdb
makedb_SOURCES = makedb.c
makedb_LDADD = libosux_db.la ../beatmap/libbeatmap.la

readdb_SOURCES = readdb.c
readdb_LDADD = libosux_db.la ../beatmap/libbeatmap.la

