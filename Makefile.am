##
#	txt2pdbdoc -- Text to Doc converter for Palm Pilots
#	Makefile.am
#
#	Copyright (C) 2001  Paul J. Lucas
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
# 
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
# 
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
##

ACLOCAL_AMFLAGS = -I m4
SUBDIRS = bin lib src man test

EXTRA_DIST =	bootstrap \
		m4/gnulib-cache.m4 \
		README.md

.PHONY:	update-gnulib

update-gnulib:
	gnulib-tool --add-import
	rm -f m4/.gitignore

# vim:set noet sw=8 ts=8:
