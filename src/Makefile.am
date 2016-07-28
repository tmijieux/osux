include $(top_srcdir)/common.mk

SUBDIRS = thirdparty util beatmap mod database replay . taikorank

lib_LTLIBRARIES = libosux.la
libosux_la_SOURCES = 
libosux_la_LIBADD = \
	beatmap/libbeatmap.la \
	replay/libreplay.la \
	util/libosux_util.la \
	database/libosux_db.la \
	mod/libmod.la
