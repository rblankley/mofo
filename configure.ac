# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.64)
AC_INIT(mofo, 0.0.1, rblankley@woh.rr.com)

AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([subdir-objects])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADER([config.h])

AC_CONFIG_MACRO_DIRS([m4])

# setup doxygen
DX_INIT_DOXYGEN([$PACKAGE_NAME], [Doxyfile], [doc])

# use the C++ compiler for the following checks
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX_17([noext], [mandatory])

AX_CHECK_COMPILE_FLAG([-pipe], [CXXFLAGS="$CXXFLAGS -pipe"], [])
AX_CHECK_COMPILE_FLAG([-Wall], [CXXFLAGS="$CXXFLAGS -Wall"], [])
AX_CHECK_COMPILE_FLAG([-Wextra], [CXXFLAGS="$CXXFLAGS -Wextra"], [])

# Silence warning: Qt5: hundreds of warnings from Qt5 regarding depreciated copy constuctors
AX_CHECK_COMPILE_FLAG([-Wno-deprecated-copy], [CXXFLAGS="$CXXFLAGS -Wno-deprecated-copy"], [])

AX_PTHREAD(
 [LIBS="$PTHREAD_LIBS $LIBS"
  CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
  CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"],
 [AC_MSG_ERROR([pthread is required to compile this repo])])

# Silence warning: ar: 'u' modifier ignored since 'D' is the default
AC_SUBST(ARFLAGS, [cr])

# distribute additional compiler and linker flags among Makefiles
# --> set and change these variables instead of CXXFLAGS or LDFLAGS (for user only)
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])

# Checks for programs.
AC_PROG_CXX
AC_PROG_RANLIB

# Checks for libraries.
AC_ARG_WITH([clio], AS_HELP_STRING([--with-clio], [Build with the CLIO logger interface]))

AS_IF([test "x$with_clio" = "xyes"], [
  PKG_CHECK_MODULES([CLIO], [clio])
  AX_CXX_CHECK_LIB([clio], [clio::logger], [], [AC_MSG_ERROR([Could not find libclio library, check LDFLAGS])])
])

AX_REQUIRE_QT([QT], [QtCore QtGui QtNetwork QtNetworkAuth QtSql QtXml QtConcurrent], [QtWidgets])
AX_INIT_QT

# Checks for header files.
AC_HEADER_STAT
AC_HEADER_STDC

AC_CHECK_HEADERS([chrono])
AC_CHECK_HEADERS([condition_variable])
AC_CHECK_HEADERS([cstdarg])
AC_CHECK_HEADERS([cstdio])
AC_CHECK_HEADERS([ctime])
AC_CHECK_HEADERS([filesystem])
AC_CHECK_HEADERS([list])
AC_CHECK_HEADERS([locale])
AC_CHECK_HEADERS([map])
AC_CHECK_HEADERS([memory])
AC_CHECK_HEADERS([mutex])
AC_CHECK_HEADERS([new])
AC_CHECK_HEADERS([iostream])
AC_CHECK_HEADERS([shared_mutex])
AC_CHECK_HEADERS([sstream])
AC_CHECK_HEADERS([stack])
AC_CHECK_HEADERS([stdexcept])
AC_CHECK_HEADERS([string])
AC_CHECK_HEADERS([thread])

AS_IF([test "x$with_clio" = "xyes"], [
  ac_save_CPPFLAGS=$CPPFLAGS
  CPPFLAGS="$CPPFLAGS $CLIO_CFLAGS"
  AC_CHECK_HEADERS([clio.h], [], [AC_MSG_ERROR([Header for libclio is missing or invalid, check CPPFLAGS])])
  CPPFLAGS=$ac_save_CPPFLAGS
])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS(localtime)
AC_CHECK_FUNCS(localtime_r)
AC_CHECK_FUNCS(stat)

# creating top level and subdirectory Makefiles:
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/apibase/Makefile
	src/calc/Makefile
	src/db/Makefile
	src/tda/Makefile
	src/usdot/Makefile
	src/util/Makefile
])

AC_OUTPUT

