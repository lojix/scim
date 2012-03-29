#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([scim],[0.0.1],[bug-scim@syncompute.net])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([gnu no-exeext dist-bzip2 no-dist-gzip -Wno-portability])
AC_CONFIG_HEADER([config.h])
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
LT_INIT
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_SYS_LARGEFILE

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC

LONG_BIT=`getconf LONG_BIT`
rootlibdir=/lib$LONG_BIT

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_SUBST([LONG_BIT])
AC_SUBST([rootlibdir])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
