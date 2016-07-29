include $(top_srcdir)/common.mk

SUBDIRS = parser

noinst_LTLIBRARIES= libosux_beatmap.la
libosux_beatmap_la_SOURCES = \
	beatmap.c \
	beatmap.h \
	color.c \
	color.h \
	hitobject.c \
	hitobject.h \
	hitsound.h \
	storyboard.c \
	storyboard.h \
	timingpoint.c \
	timingpoint.h \
	taiko_autoconvert.c \
	taiko_autoconvert.h

libosux_beatmap_la_LIBADD = \
	parser/libosux_parser.la
