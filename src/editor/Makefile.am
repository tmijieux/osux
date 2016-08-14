include $(top_srcdir)/common.mk

BUILT_SOURCES = resources.c resources.h
bin_PROGRAMS = osux_editor
osux_editor_SOURCES = \
	main.c \
	app.c \
	app_win.c \
	popover_search_bar.c \
	resources.c

GLIB_BUILD_RESOURCE = glib-compile-resources

resources.h: 
	$(GLIB_BUILD_RESOURCE) $(srcdir)/editor.gresources.xml \
	--sourcedir=$(srcdir) \
	--target=$@ \
	--generate-header

resources.c: resources.h
	$(GLIB_BUILD_RESOURCE) $(srcdir)/editor.gresources.xml \
	--sourcedir=$(srcdir) \
	--target=$@ \
	--generate-source


osux_editor_CFLAGS = $(GTK3_CFLAGS) $(AM_CFLAGS)
osux_editor_LDFLAGS = $(GTK3_LIBS) -export-dynamic $(AM_LDFLAGS)
