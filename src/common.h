/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      common.h
**
**      Copyright (C) 1998-2024  Paul J. Lucas
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

#ifndef txt2pdbdoc_common_H
#define txt2pdbdoc_common_H

// local
#include "palm.h"

///////////////////////////////////////////////////////////////////////////////

#define BUFFER_SIZE               6000  /* big enough for uncompressed record */

#define NEW_BUFFER(B) (B)->data = MALLOC( Byte, (B)->len = BUFFER_SIZE )

struct buffer {
  Byte   *data;
  size_t  len;
};
typedef struct buffer buffer_t;

///////////////////////////////////////////////////////////////////////////////

#endif /* txt2pdbdoc_common_H */
/* vim:set et sw=2 ts=2: */
