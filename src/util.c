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
#include <ctype.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////
  
/** 
 * A node for a singly linked list of pointers to memory to be freed via
 * \c atexit().
 */
struct free_node {
  void *ptr;
  struct free_node *next;
};
typedef struct free_node free_node_t;

////////// local variables ////////////////////////////////////////////////////

static free_node_t *free_head;          // linked list of stuff to free

////////// local functions ////////////////////////////////////////////////////

/**
 * Skips leading whitespace, if any.
 *
 * @param s The NULL-terminated string to skip whitespace for.
 * @return Returns a pointer within \a s pointing to the first non-whitespace
 * character or pointing to the NULL byte if either \a s was all whitespace or
 * empty.
 */
static char const* skip_ws( char const *s ) {
  assert( s );
  while ( isspace( *s ) )
    ++s;
  return s;
}

////////// extern functions ///////////////////////////////////////////////////

FILE* check_fopen( char const *path, char const *mode ) {
  assert( path );
  assert( mode );
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

char* check_strdup( char const *s ) {
  assert( s );
  char *const dup = strdup( s );
  if ( !dup )
    PERROR_EXIT( OUT_OF_MEMORY );
  return dup;
}

void* freelist_add( void *p ) {
  assert( p );
  free_node_t *const new_node = MALLOC( free_node_t, 1 );
  new_node->ptr = p;
  new_node->next = free_head ? free_head : NULL;
  free_head = new_node;
  return p;
}

void freelist_free() {
  for ( free_node_t *p = free_head; p; ) {
    free_node_t *const next = p->next;
    free( p->ptr );
    free( p );
    p = next;
  } // for
  free_head = NULL;
}

uint8_t* mem_find( uint8_t *t, size_t t_len, uint8_t *m, size_t m_len ) {
  assert( t );
  assert( m );

  for ( size_t i = t_len - m_len + 1; i > 0; --i, ++t )
    if ( *t == *m && !memcmp( t, m, m_len ) )
      return t;
  return NULL;
}

uint64_t parse_ull( char const *s ) {
  assert( s );
  s = skip_ws( s );
  if ( *s && *s != '-') {               // strtoull(3) wrongly allows '-'
    char *end = NULL;
    errno = 0;
    uint64_t const n = strtoull( s, &end, 0 );
    if ( !errno && !*end )
      return n;
  }
  PMESSAGE_EXIT( USAGE, "\"%s\": invalid integer\n", s );
}

int peekc( FILE *file ) {
  int const c = getc( file );
  if ( c == EOF ) {
    if ( ferror( file ) )
      PERROR_EXIT( READ_ERROR );
  } else {
    UNGETC( c, file );
  }
  return c;
}

char const* printable_char( char c ) {
  switch( c ) {
    case '\0': return "\\0";
    case '\a': return "\\a";
    case '\b': return "\\b";
    case '\f': return "\\f";
    case '\n': return "\\n";
    case '\r': return "\\r";
    case '\t': return "\\t";
    case '\v': return "\\v";
  } // switch

  static char buf[5];                   // \xHH + NULL
  if ( isprint( c ) )
    buf[0] = c, buf[1] = '\0';
  else
    snprintf( buf, sizeof( buf ), "\\x%02X", (unsigned)c );
  return buf;
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
