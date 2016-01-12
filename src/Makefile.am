include $(top_srcdir)/common.mk

SUBDIRS = util beatmap mod replay . #editor 
lib_LTLIBRARIES = libosux.la
libosux_la_SOURCES = python.c
libosux_la_LIBADD = \
	beatmap/libbeatmap.la \
	replay/libreplay.la 
	util/libutil.la
	mod/libmod.la

#editor/libeditor.la
