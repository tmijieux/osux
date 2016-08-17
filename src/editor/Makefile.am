include $(top_srcdir)/common.mk

bin_PROGRAMS = osux_editor
osux_editor_SOURCES = \
	main.c \
	app.c \
	app_win.c \
	popsearch.c \
	closelabel.c \
	adjustment.c \
	beatmap.c

nodist_osux_editor_SOURCES = \
	resources.c \
	resources.h

osux_editor.$(OBJEXT): resources.c resources.h

CLEANFILES = \
	resources.c \
	resources.h

GLIB_BUILD_RESOURCE = glib-compile-resources
RESOURCES_DEP= \
	$(srcdir)/ui/OsuxEditorWindow.glade \
	$(srcdir)/ui/OsuxEditorPopover.glade \
	$(srcdir)/ui/OsuxBeatmapAdjustments.glade \
	$(srcdir)/ui/OsuxCloseLabel.glade \
	$(srcdir)/ui/OsuxFileFilterMusic.glade

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
