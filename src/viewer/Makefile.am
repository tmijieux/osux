include $(top_srcdir)/common.mk

bin_PROGRAMS = vosu
vosu_SOURCES = \
	main.c \
	vosu-app.c vosu-app.h \
	vosu-win.c vosu-win.h \
	vosu-beatmap.c vosu-beatmap.h \
	vosu-player.c vosu-player.h \
	vosu-view.c vosu-view.h \
	vosu-replay.c vosu-replay.h \
	vosu-sequence.c vosu-sequence.h \
	vosu-render.c vosu-render.h \
	vosu-render-taiko.c \
	vosu-render-std.c

nodist_vosu_SOURCES = \
	resources.c \
	resources.h

osux_editor.$(OBJEXT): resources.c resources.h

CLEANFILES = \
	resources.c \
	resources.h

GLIB_BUILD_RESOURCE = glib-compile-resources
RESOURCES_DEP= \
	$(srcdir)/ui/VosuWindow.glade \
	$(srcdir)/ui/VosuView.glade \
	$(srcdir)/ui/VosuFileFilterMusic.glade \
	$(srcdir)/ui/VosuFileFilterOsu.glade

$(srcdir)/editor.gresources.xml: $(RESOURCES_DEP)
# technically nothing can be 'rebuilt' here so just update modification time
	touch $@

resources.h: $(srcdir)/editor.gresources.xml
	$(GLIB_BUILD_RESOURCE) $(srcdir)/editor.gresources.xml \
	--sourcedir=$(srcdir) \
	--target=$@ \
	--generate-header

resources.c: resources.h $(srcdir)/editor.gresources.xml
	$(GLIB_BUILD_RESOURCE) $(srcdir)/editor.gresources.xml \
	--sourcedir=$(srcdir) \
	--target=$@ \
	--generate-source

vosu_CFLAGS = $(GTK3_CFLAGS) $(EPOXY_CFLAGS) $(AM_CFLAGS) $(GST_CFLAGS)
vosu_LDFLAGS = $(GTK3_LIBS) $(EPOXY_LIBS) -export-dynamic $(AM_LDFLAGS) -lm $(GST_LIBS)
vosu_LDADD = ../lib/libosux.la
