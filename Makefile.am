include $(top_srcdir)/common.mk

ACLOCAL_AMFLAGS = -I m4
dist_doc_DATA = README.md

SUBDIRS = util python beatmap mod database replay . taikorank

lib_LTLIBRARIES = libosux.la
libosux_la_SOURCES = 
libosux_la_LIBADD = \
	python/libembedpython.la \
	beatmap/libbeatmap.la \
	replay/libreplay.la \
	util/libosux_util.la \
	database/libosux_db.la \
	mod/libmod.la
