include $(top_srcdir)/common.mk

SUBDIRS = util beatmap mod database replay

lib_LTLIBRARIES = libosux.la
libosux_la_SOURCES = dummy.c
libosux_la_LIBADD = \
	util/libosux_util.la \
	beatmap/libosux_beatmap.la \
	mod/libosux_mod.la \
	database/libosux_db.la \
	replay/libosux_replay.la

