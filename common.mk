pkgconfdir=$(sysconfdir)/$(PACKAGE)
yamldir = $(pkgconfdir)/yaml
pythondir = $(pkglibdir)
AM_CFLAGS = \
	-Wall -Wextra \
	-I$(top_srcdir)/src/include \
	-DPKG_DATA_DIR=\"$(pkgdatadir)\" \
	-DPKG_CONFIG_DIR=\"$(pkgconfdir)\" \
	-std=gnu99 \
	$(GLIB_CFLAGS) $(GMODULE_CFLAGS) \
	$(GOBJECT_CFLAGS) $(GIO_CFLAGS)

AM_LDFLAGS = \
	$(GLIB_LIBS) $(GMODULE_LIBS) \
	$(GOBJECT_LIBS) $(GIO_LIBS) \
	-no-undefined
