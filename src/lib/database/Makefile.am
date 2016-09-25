include $(top_srcdir)/common.mk

BUILT_SOURCES = beatmap_db.sql.c beatmap_db.sql.h

noinst_LTLIBRARIES = libosux_db.la
libosux_db_la_SOURCES = \
	beatmap_database.c database.c \
	beatmap_db.sql

nodist_libosux_db_la_SOURCES = \
	beatmap_db.sql.c \
	beatmap_db.sql.h

beatmap_db.sql.c beatmap_db.sql.h:
	$(top_srcdir)/scripts/embedder2.py $(srcdir)/beatmap_db.sql

CLEAN_FILES = _db_sql_data.c

libosux_db_la_CFLAGS = $(SQLITE_CFLAGS) $(AM_CFLAGS)
libosux_db_la_LDFLAGS = $(SQLITE_LIBS) $(AM_LDFLAGS)

