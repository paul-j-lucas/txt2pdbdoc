/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      common.h
**
**      Copyright (C) 1998-2015  Paul J. Lucas
**
**      This program is free software; you can redistribute it and/or modify
**      it under the terms of the GNU General Public License as published by
**      the Free Software Foundation; either version 2 of the License, or
**      (at your option) any later version.
** 
**      This program is distributed in the hope that it will be useful,
**      but WITHOUT ANY WARRANTY; without even the implied warranty of
**      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**      GNU General Public License for more details.
** 
**      You should have received a copy of the GNU General Public License
**      along with this program; if not, write to the Free Software
**      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef txt2pdbdoc_H
#define txt2pdbdoc_H

// standard
#include <stdlib.h>                     /* for EXIT_SUCCESS */

///////////////////////////////////////////////////////////////////////////////

#define EXIT_USAGE                1     /* command-line usage error */
#define EXIT_OUT_OF_MEMORY        2
#define EXIT_OPEN_ERROR           10    /* error opening file */
#define EXIT_READ_ERROR           11    /* error reading */
#define EXIT_WRITE_ERROR          12    /* error writing */
#define EXIT_SEEK_ERROR           13    /* error seeking */
#define EXIT_NOT_DOC_FILE         20
#define EXIT_UNKNOWN_COMPRESSION  21

///////////////////////////////////////////////////////////////////////////////

#endif /* txt2pdbdoc_H */
/* vim:set et sw=2 ts=2: */
