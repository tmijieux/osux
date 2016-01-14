include $(top_srcdir)/common.mk

SUBDIRS = util beatmap mod replay . ranking/taikorank
lib_LTLIBRARIES = libosux.la
libosux_la_SOURCES = python.c
libosux_la_LIBADD = \
	beatmap/libbeatmap.la \
	replay/libreplay.la 
	util/libosux_util.la
	mod/libmod.la

libosux_la_CFLAGS = $(PYTHON2_CFLAGS) $(AM_CFLAGS)
libosux_la_LDFLAGS = $(PYTHON2_LIBS) $(AM_LDFLAGS)

#editor/libeditor.la
