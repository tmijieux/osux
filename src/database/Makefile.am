include $(top_srcdir)/common.mk

BUILT_SOURCES = _db_sql_data.c
noinst_LTLIBRARIES = libosux_db.la
libosux_db_la_SOURCES = osux_db.c _db_sql_data.c db.sql

_db_sql_data.c:
	$(top_srcdir)/src/scripts/embedder.py $(srcdir)/db.sql > $@

CLEAN_FILES = _db_sql_data.c

libosux_db_la_LIBADD = ../util/libosux_util.la

libosux_db_la_CFLAGS = $(SQLITE_CFLAGS) $(AM_CFLAGS)
libosux_db_la_LDFLAGS = $(SQLITE_LIBS) $(AM_LDFLAGS)

bin_PROGRAMS = makedb readdb
makedb_SOURCES = makedb.c
makedb_LDADD = libosux_db.la ../beatmap/libbeatmap.la

readdb_SOURCES = readdb.c
readdb_LDADD = libosux_db.la ../beatmap/libbeatmap.la

