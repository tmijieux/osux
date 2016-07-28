AC_INIT([osux], [0.1], [lmaugere@ipb.fr, tmijieux@ipb.fr])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_SILENT_RULES([yes])
AM_PROG_AR
LT_INIT([win32-dll dlopen disable-static])
AC_CONFIG_MACRO_DIRS([m4])
AC_PROG_CC

PKG_CHECK_MODULES(YAML, yaml-0.1) 
PKG_CHECK_MODULES(LZMA, liblzma) 
PKG_CHECK_MODULES(PYTHON2, python2) 
PKG_CHECK_MODULES(CRYPTO, libcrypto)
PKG_CHECK_MODULES(SQLITE, sqlite3)
PKG_CHECK_MODULES([GLIB], [glib-2.0])
PKG_CHECK_MODULES([GMODULE], [gmodule-2.0])
PKG_CHECK_MODULES(GTS, gts >= 0.7.2, ,
                       [AC_CHECK_PROG(GTS_CONFIG, gts-config,
                       gts-config, false)
		       if test "x$GTS_CONFIG" = "xfalse" ; then
                          AC_MSG_ERROR([pkg/gts-config not in path,
                       aborting])
                       fi
                       GTS_CFLAGS=$(${GTS_CONFIG} --cflags)
                       GTS_LIBS=$(${GTS_CONFIG} --libs)])

AC_SUBST(GTS_CFLAGS)
AC_SUBST(GTS_LIBS)

AC_CONFIG_FILES([
		  Makefile
		  src/Makefile
		  src/thirdparty/Makefile
		  src/util/Makefile
		  src/mod/Makefile
		  src/beatmap/Makefile
		  src/beatmap/parser/Makefile
      		  src/beatmap/parser/parser_py/Makefile
 		  src/beatmap/parser/parser_c/Makefile
		  src/editor/Makefile
		  src/replay/Makefile
		  src/database/Makefile
		  src/taikorank/Makefile
		  src/taikorank/yaml/Makefile
])
# AC_CONFIG_SUBDIRS([src/thirdparty/libyaml
#                   src/thirdparty/xz
#                    ])


AC_OUTPUT
