include $(top_srcdir)/common.mk

bin_PROGRAMS = edosu
edosu_SOURCES = \
	main.c \
	edosu-app.c edosu-app.h \
	edosu-win.c edosu-win.h \
	edosu-close-lbl.c edosu-close-lbl.h \
	edosu-beatmap.c edosu-beatmap.h \
	edosu-beatmap-objects.c \
	edosu-properties.c edosu-properties.h \
	edosu-palette.c edosu-palette.h \
	edosu-view.c edosu-view.h \
	edosu-inspector.c edosu-inspector.h \
	edosu-render.c edosu-render.h

nodist_edosu_SOURCES = \
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

edosu_CFLAGS = $(GTK3_CFLAGS) $(EPOXY_CFLAGS) $(AM_CFLAGS)
edosu_LDFLAGS = $(GTK3_LIBS) $(EPOXY_LIBS) -export-dynamic $(AM_LDFLAGS) -lm
edosu_LDADD = ../lib/libosux.la
