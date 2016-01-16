include $(top_srcdir)/common.mk

SUBDIRS = util beatmap mod replay database . ranking/taikorank
lib_LTLIBRARIES = libosux.la
libosux_la_SOURCES = python.c
libosux_la_LIBADD = \
	beatmap/libbeatmap.la \
	replay/libreplay.la \
	util/libosux_util.la \
	database/libosux_db.la

#mod/libmod.la

libosux_la_CFLAGS = $(PYTHON2_CFLAGS) $(AM_CFLAGS) -DPYTHON_PATH=\"$(pythondir)\"
libosux_la_LDFLAGS = $(PYTHON2_LIBS) $(AM_LDFLAGS)

#editor/libeditor.la
