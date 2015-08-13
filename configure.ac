# Process this file with autoconf to produce a configure script.

##
#     txt2pdbdoc -- Text to Doc converter for Palm Pilots
#     configure.ac
#
#     Copyright (C) 2015  Paul J. Lucas
#
#     This program is free software; you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation; either version 2 of the Licence, or
#     (at your option) any later version.
#     
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#     
#     You should have received a copy of the GNU General Public License
#     along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

AC_INIT([txt2pdbdoc], [1.6], [paul@lucasmail.org],,[https://github.com/paul-j-lucas/txt2pdbdoc])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checks for programs.
AC_PROG_CC_C99
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

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_FSEEKO
AC_FUNC_REALLOC
AC_CHECK_FUNCS([perror strerror])

# Miscellaneous.
AC_LANG(C)
AX_CFLAGS_WARN_ALL
AX_CHECK_COMPILE_FLAG([-Wextra],[CFLAGS="$CFLAGS -Wextra"])
AX_CHECK_COMPILE_FLAG([-Wc++-compat],[CFLAGS="$CFLAGS -Wc++-compat"])
AX_CHECK_COMPILE_FLAG([-Wredundant-decls],[CFLAGS="$CFLAGS -Wredundant-decls"])
AX_CHECK_COMPILE_FLAG([-Wwrite-strings],[CFLAGS="$CFLAGS -Wwrite-strings"])

# Generate files.
AH_TOP([#ifndef txt2pdbdoc_config_H
#define txt2pdbdoc_config_H])
AH_BOTTOM([#endif /* txt2pdbdoc_config_H */])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([
  Makefile
  src/Makefile
  man/Makefile
  man/man1/Makefile
  man/man4/Makefile
])
AC_OUTPUT

# vim:set et sw=2 ts=2:
