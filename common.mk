pkgconfdir=$(sysconfdir)/$(PACKAGE)
yamldir = $(pkgconfdir)/yaml
pythondir = $(pkglibdir)
AM_CFLAGS = \
	-Wall -Wextra \
	-I$(top_srcdir)/src/include \
	-std=gnu99 \
	$(GLIB_CFLAGS) $(GMODULE_CFLAGS) \
	$(GOBJECT_CFLAGS) $(GIO_CFLAGS)

#preprocessor:
AM_CPPFLAGS= \
    -DLOCALEDIR=\""$(localedir)"\" \
    -DPKG_DATA_DIR=\"$(pkgdatadir)\" \
    -DPKG_CONFIG_DIR=\"$(pkgconfdir)\"

AM_LDFLAGS = \
	$(GLIB_LIBS) $(GMODULE_LIBS) \
	$(GOBJECT_LIBS) $(GIO_LIBS) \
	-no-undefined
