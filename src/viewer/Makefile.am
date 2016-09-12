include $(top_srcdir)/common.mk

bin_PROGRAMS = vosu
vosu_SOURCES = \
	main.c \
	vosu-app.c \
	vosu-win.c \
	vosu-close-lbl.c \
	vosu-beatmap.c \
	vosu-beatmap-objects.c \
	vosu-view.c \
	vosu-render.c

nodist_vosu_SOURCES = \
	resources.c \
	resources.h

osux_editor.$(OBJEXT): resources.c resources.h

CLEANFILES = \
	resources.c \
	resources.h

GLIB_BUILD_RESOURCE = glib-compile-resources
RESOURCES_DEP= \
	$(srcdir)/ui/vosuWindow.glade \
	$(srcdir)/ui/vosuView.glade \
	$(srcdir)/ui/vosuInspector.glade \
	$(srcdir)/ui/vosuPalette.glade \
	$(srcdir)/ui/vosuProperties.glade \
	$(srcdir)/ui/vosuCloseLabel.glade \
	$(srcdir)/ui/vosuFileFilterMusic.glade \
	$(srcdir)/ui/vosuFileFilterOsu.glade

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

vosu_CFLAGS = $(GTK3_CFLAGS) $(EPOXY_CFLAGS) $(AM_CFLAGS)
vosu_LDFLAGS = $(GTK3_LIBS) $(EPOXY_LIBS) -export-dynamic $(AM_LDFLAGS) -lm
vosu_LDADD = ../lib/libosux.la
