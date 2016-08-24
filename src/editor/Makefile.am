include $(top_srcdir)/common.mk

bin_PROGRAMS = osux_editor
osux_editor_SOURCES = \
	main.c \
	edosu-app.c \
	edosu-win.c \
	edosu-close-lbl.c \
	edosu-adjust.c \
	edosu-beatmap.c \
	edosu-beatmap-load.c \
	edosu-properties.c \
	edosu-palette.c \
	edosu-view.c \
	edosu-inspector.c

nodist_osux_editor_SOURCES = \
	resources.c \
	resources.h

osux_editor.$(OBJEXT): resources.c resources.h

CLEANFILES = \
	resources.c \
	resources.h

GLIB_BUILD_RESOURCE = glib-compile-resources
RESOURCES_DEP= \
	$(srcdir)/ui/EdosuWindow.glade \
	$(srcdir)/ui/EdosuView.glade \
	$(srcdir)/ui/EdosuInspector.glade \
	$(srcdir)/ui/EdosuPalette.glade \
	$(srcdir)/ui/EdosuProperties.glade \
	$(srcdir)/ui/EdosuBeatmapAdjustments.glade \
	$(srcdir)/ui/EdosuCloseLabel.glade \
	$(srcdir)/ui/EdosuFileFilterMusic.glade \
	$(srcdir)/ui/EdosuFileFilterOsu.glade

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

osux_editor_CFLAGS = $(GTK3_CFLAGS) $(AM_CFLAGS)
osux_editor_LDFLAGS = $(GTK3_LIBS) -export-dynamic $(AM_LDFLAGS)
osux_editor_LDADD = ../lib/libosux.la
