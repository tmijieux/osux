pkgconfdir=$(sysconfdir)/$(PACKAGE)
yamldir = $(pkgconfdir)/yaml
pythondir = $(pkglibdir)
AM_CFLAGS = -fPIC -Wall -Wextra -I$(top_srcdir)/src -I$(top_srcdir)/src/util \
	    -I$(top_srcdir)/src/include -DPKG_DATA_DIR=\"$(pkgdatadir)\" \
	    -DPKG_CONFIG_DIR=\"$(pkgconfdir)\" -g -ggdb -O3 -std=gnu99
AM_LDFLAGS=-rdynamic

