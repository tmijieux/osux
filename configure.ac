AC_INIT([osux], [0.1], [lmaugere@ipb.fr, tmijieux@ipb.fr])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_SILENT_RULES([yes])
AM_PROG_AR
LT_INIT([dlopen disable-static])
AC_CONFIG_MACRO_DIRS([m4])
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])

PKG_CHECK_MODULES(PYTHON2, python2) 
PKG_CHECK_MODULES(YAML, yaml-0.1) 
PKG_CHECK_MODULES(LZMA, liblzma) 
PKG_CHECK_MODULES(CRYPTO, libcrypto)
PKG_CHECK_MODULES(SQLITE, sqlite3) 

AC_CONFIG_FILES([
		  Makefile
		  util/Makefile
		  python/Makefile
		  mod/Makefile
		  beatmap/Makefile
		  beatmap/parser/Makefile
		  editor/Makefile
		  replay/Makefile
		  database/Makefile
		  taikorank/Makefile
		  taikorank/yaml/Makefile
])

AC_OUTPUT
