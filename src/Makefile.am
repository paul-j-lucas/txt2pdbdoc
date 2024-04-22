##
#	txt2pdbdoc -- Text to Doc converter for Palm Pilots
#	src/Makefile.am
#
#	Copyright (C) 2015-2024  Paul J. Lucas
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the Licence, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

bin_PROGRAMS =		txt2pdbdoc pdbdump

AM_CFLAGS =		$(T2PD_CFLAGS)
AM_CPPFLAGS =		-I$(top_srcdir)/lib -I$(top_builddir)/lib

pdbdump_SOURCES =	common.h \
			pdbdump.c \
			pjl_config.h \
			options.c options.h \
			util.c util.h

txt2pdbdoc_SOURCES =	common.h \
			compress.c \
			decode.c \
			encode.c \
			options.c options.h \
			palm.c palm.h \
			pjl_config.h \
			txt2pdbdoc.c \
			unicode.c unicode.h \
			util.c util.h

# vim:set noet sw=8 ts=8:
