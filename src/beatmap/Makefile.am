include $(top_srcdir)/common.mk

SUBDIRS = parser

noinst_LTLIBRARIES= libbeatmap.la
libbeatmap_la_SOURCES = \
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

libbeatmap_la_LIBADD = parser/libosux_parser.la 

