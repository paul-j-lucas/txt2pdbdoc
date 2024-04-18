# Process this file with autoconf to produce a configure script.

##
#       txt2pdbdoc -- Text to Doc converter for Palm Pilots
#       configure.ac
#
#       Copyright (C) 2015-2024  Paul J. Lucas
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 2 of the Licence, or
#       (at your option) any later version.
#       
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#       
#       You should have received a copy of the GNU General Public License
#       along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

AC_INIT([txt2pdbdoc], [1.5], [paul@lucasmail.org],,[https://github.com/paul-j-lucas/txt2pdbdoc])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checks for programs.
AC_LANG(C)
AC_PROG_CC
gl_EARLY
AC_PROG_INSTALL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([errno.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([stddef.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([time.h])
AC_CHECK_HEADERS([unistd.h])
AC_HEADER_ASSERT
AC_HEADER_STDBOOL
gl_INIT

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
PJL_CHECK_TYPEDEF(char8_t,[uchar.h],
  AC_DEFINE([HAVE_CHAR8_T], [1], [Define to 1 if `char8_t' is supported.]),
  AC_DEFINE([HAVE_CHAR8_T], [0], [Define to 1 if `char8_t' is supported.])
)
PJL_CHECK_TYPEDEF(char16_t,[uchar.h],
  AC_DEFINE([HAVE_CHAR16_T], [1], [Define to 1 if `char16_t' is supported.]),
  AC_DEFINE([HAVE_CHAR16_T], [0], [Define to 1 if `char16_t' is supported.])
)
PJL_CHECK_TYPEDEF(char32_t,[uchar.h],
  AC_DEFINE([HAVE_CHAR32_T], [1], [Define to 1 if `char32_t' is supported.]),
  AC_DEFINE([HAVE_CHAR32_T], [0], [Define to 1 if `char32_t' is supported.])
)

# Checks for library functions.
AC_FUNC_FSEEKO
AC_FUNC_REALLOC
AC_CHECK_FUNCS([perror strerror])

# Compiler warnings
AC_SUBST([T2PD_CFLAGS])
T2PD_CFLAGS="-D_ISOC11_SOURCE"
AX_CFLAGS_WARN_ALL([T2PD_CFLAGS])
AX_CHECK_COMPILE_FLAG([-Wcast-align], [T2PD_CFLAGS="$T2PD_CFLAGS -Wcast-align"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wcast-function-type], [T2PD_CFLAGS="$T2PD_CFLAGS -Wcast-function-type"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wcomma], [T2PD_CFLAGS="$T2PD_CFLAGS -Wcomma"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wconditional-type-mismatch], [T2PD_CFLAGS="$T2PD_CFLAGS -Wconditional-type-mismatch"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wconditional-uninitialized], [T2PD_CFLAGS="$T2PD_CFLAGS -Wconditional-uninitialized"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wconversion], [T2PD_CFLAGS="$T2PD_CFLAGS -Wconversion"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wduplicate-enum], [T2PD_CFLAGS="$T2PD_CFLAGS -Wduplicate-enum"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wembedded-directive], [T2PD_CFLAGS="$T2PD_CFLAGS -Wembedded-directive"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wenum-enum-conversion], [T2PD_CFLAGS="$T2PD_CFLAGS -Wenum-enum-conversion"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wenum-float-conversion], [T2PD_CFLAGS="$T2PD_CFLAGS -Wenum-float-conversion"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wextra], [T2PD_CFLAGS="$T2PD_CFLAGS -Wextra"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wfloat-equal], [T2PD_CFLAGS="$T2PD_CFLAGS -Wfloat-equal"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wfor-loop-analysis], [T2PD_CFLAGS="$T2PD_CFLAGS -Wfor-loop-analysis"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wformat-nonliteral], [T2PD_CFLAGS="$T2PD_CFLAGS -Wformat-nonliteral"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wformat-signedness], [T2PD_CFLAGS="$T2PD_CFLAGS -Wformat-signedness"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wformat-type-confusion], [T2PD_CFLAGS="$T2PD_CFLAGS -Wformat-type-confusion"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wformat=2], [T2PD_CFLAGS="$T2PD_CFLAGS -Wformat=2"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Widiomatic-parentheses], [T2PD_CFLAGS="$T2PD_CFLAGS -Widiomatic-parentheses"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wimplicit-fallthrough], [T2PD_CFLAGS="$T2PD_CFLAGS -Wimplicit-fallthrough"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wlogical-op-parentheses], [T2PD_CFLAGS="$T2PD_CFLAGS -Wlogical-op-parentheses"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wmisleading-indentation], [T2PD_CFLAGS="$T2PD_CFLAGS -Wmisleading-indentation"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wnewline-eof], [T2PD_CFLAGS="$T2PD_CFLAGS -Wnewline-eof"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wpedantic], [T2PD_CFLAGS="$T2PD_CFLAGS -Wpedantic"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wredundant-decls], [T2PD_CFLAGS="$T2PD_CFLAGS -Wredundant-decls"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wshadow], [T2PD_CFLAGS="$T2PD_CFLAGS -Wshadow"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wshift-sign-overflow], [T2PD_CFLAGS="$T2PD_CFLAGS -Wshift-sign-overflow"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wshorten-64-to-32], [T2PD_CFLAGS="$T2PD_CFLAGS -Wshorten-64-to-32"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wsign-compare], [T2PD_CFLAGS="$T2PD_CFLAGS -Wsign-compare"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wsign-conversion], [T2PD_CFLAGS="$T2PD_CFLAGS -Wsign-conversion"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wsometimes-uninitialized], [T2PD_CFLAGS="$T2PD_CFLAGS -Wsometimes-uninitialized"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wstring-conversion], [T2PD_CFLAGS="$T2PD_CFLAGS -Wstring-conversion"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wtautological-compare], [T2PD_CFLAGS="$T2PD_CFLAGS -Wtautological-compare"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wtautological-type-limit-compare], [T2PD_CFLAGS="$T2PD_CFLAGS -Wtautological-type-limit-compare"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wundef], [T2PD_CFLAGS="$T2PD_CFLAGS -Wundef"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wuninitialized], [T2PD_CFLAGS="$T2PD_CFLAGS -Wuninitialized"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wunreachable-code-break], [T2PD_CFLAGS="$T2PD_CFLAGS -Wunreachable-code-break"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wunreachable-code-return], [T2PD_CFLAGS="$T2PD_CFLAGS -Wunreachable-code-return"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wunreachable-code], [T2PD_CFLAGS="$T2PD_CFLAGS -Wunreachable-code"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wunused], [T2PD_CFLAGS="$T2PD_CFLAGS -Wunused"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wwrite-strings], [T2PD_CFLAGS="$T2PD_CFLAGS -Wwrite-strings"], [], [-Werror])
AX_CHECK_COMPILE_FLAG([-Wzero-as-null-pointer-constant], [T2PD_CFLAGS="$T2PD_CFLAGS -Wzero-as-null-pointer-constant"], [], [-Werror])

# Generate files.
AH_TOP([#ifndef txt2pdbdoc_config_H
#define txt2pdbdoc_config_H])
AH_BOTTOM([#endif /* txt2pdbdoc_config_H */])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([
  Makefile
  lib/Makefile
  src/Makefile
  bin/Makefile
  test/Makefile
  man/Makefile
  man/man1/Makefile
  man/man4/Makefile
])
AC_OUTPUT

# vim:set et sw=2 ts=2:
