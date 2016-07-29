include $(top_srcdir)/common.mk

BUILT_SOURCES = _db_sql_data.c
noinst_LTLIBRARIES = libosux_db.la
libosux_db_la_SOURCES = osux_db.c _db_sql_data.c db.sql

_db_sql_data.c:
	$(top_srcdir)/scripts/embedder.py $(srcdir)/db.sql > $@

CLEAN_FILES = _db_sql_data.c

libosux_db_la_CFLAGS = $(SQLITE_CFLAGS) $(AM_CFLAGS)
libosux_db_la_LDFLAGS = $(SQLITE_LIBS) $(AM_LDFLAGS)

