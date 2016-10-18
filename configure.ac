#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([lifutils], [1.7.1], [bug400@gmx.de])
AM_INIT_AUTOMAKE
LT_INIT(disable_shared)
AC_CONFIG_SRCDIR([src/progs/wcat41.c])
AC_CONFIG_HEADERS([config.h])
case $host in
*darwin*)
	MACOSX=yes
	;;
*linux*)
	LINUX=yes
	;;
*mingw*)
	WINDOWS=yes
	MINGW=yes
	;;
esac

AM_CONDITIONAL(LINUX, test x$LINUX = xyes)
AM_CONDITIONAL(MACOSX, test x$MACOSX = xyes)
AM_CONDITIONAL(WINDOWS, test x$WINDOWS = xyes)
AM_CONDITIONAL(MINGW, test x$MINGW = xyes)

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h stdlib.h string.h sys/ioctl.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([ftruncate memset select strcasecmp strchr strerror strtol])

AC_CONFIG_FILES([Makefile
                 man/Makefile
                 scripts/Makefile
                 xroms/Makefile
                 doc/Makefile
                 doc/html/Makefile
                 doc/hardware/Makefile
                 src/Makefile
                 src/lib/Makefile
                 src/emu7470/Makefile
                 src/progs/Makefile])
AC_OUTPUT
