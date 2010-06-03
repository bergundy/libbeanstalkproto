#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([libbeanstalkclient], [0.1], [roey.berman@gmail.com])
AC_CONFIG_SRCDIR([src/beanstalkclient.h])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([color-tests foreign -Wall -Werror])
PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])

LT_INIT

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AM_PROG_CC_C_O



AC_SUBST(LIBTOOL_DEPS)

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([strchr strdup strndup strtoul])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tests/Makefile])

AC_CONFIG_MACRO_DIR([m4])

AC_OUTPUT
