/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      util.c
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

// local
#include "config.h"
#include "common.h"
#include "util.h"

// standard
#include <assert.h>

////////// extern functions ///////////////////////////////////////////////////

FILE* check_fopen( char const *path, char const *mode ) {
  assert( path );
  FILE *const file = fopen( path, mode );
  if ( !file )
    PMESSAGE_EXIT( OPEN_ERROR,
      "\"%s\": can not open: %s\n", path, STRERROR
    );
  return file;
}

void* check_realloc( void *p, size_t size ) {
  //
  // Autoconf, 5.5.1:
  //
  // realloc
  //    The C standard says a call realloc(NULL, size) is equivalent to
  //    malloc(size), but some old systems don't support this (e.g., NextStep).
  //
  if ( !size )
    size = 1;
  void *const r = p ? realloc( p, size ) : malloc( size );
  if ( !r )
    PERROR_EXIT( OUT_OF_MEMORY );
  return r;
}

uint8_t* mem_find( uint8_t *t, size_t t_len, uint8_t *m, size_t m_len ) {
  for ( size_t i = t_len - m_len + 1; i > 0; --i, ++t )
    if ( *t == *m && !memcmp( t, m, m_len ) )
      return t;
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
