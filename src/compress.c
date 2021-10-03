/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      compress.c
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
#include "common.h"
#include "util.h"

// standard
#include <assert.h>
#include <stdlib.h>

// constants
#define BUFFER_SIZE   6000              /* big enough for uncompressed record */
#define COUNT_BITS    3                 /* why this value?  I don't know */
#define DISP_BITS     11                /* ditto */

////////// local functions ////////////////////////////////////////////////////

/**
 * Puts a byte into a buffer.
 *
 * @param b The buffer to be affected.
 * @param c The byte.
 * @param space Is it a space?
 */
static void put_byte( buffer_t *b, Byte c, bool *space ) {
  assert( b != NULL );
  assert( space != NULL );

  if ( *space ) {
    *space = false;
    //
    // There is an outstanding space char: see if we can squeeze it in with an
    // ASCII char.
    //
    if ( c >= 0x40 && c <= 0x7F ) {
      b->data[ b->len++ ] = c ^ 0x80;
      return;
    }
    b->data[ b->len++ ] = ' ';          // couldn't squeeze it in
  } else if ( c == ' ' ) {
    *space = true;
    return;
  }

  if ( (c >= 1 && c <= 8) || c >= 0x80 )
    b->data[ b->len++ ] = '\1';

  b->data[ b->len++ ] = c;
}

////////// extern functions ///////////////////////////////////////////////////

/**
 * Replaces the given buffer with a compressed version of itself.  I don't
 * understand this algorithm.  I just cleaned up the code.
 *
 * @param b The buffer to be compressed.
 */
void compress( buffer_t *b ) {
  assert( b != NULL );

  bool space = false;

  Byte *buf_orig;
  Byte *p;                  // walking test hit; works up on successive matches
  Byte *p_prev;
  Byte *head;                           // current test string

  p = p_prev = head = buf_orig = b->data;
  Byte *tail = head + 1;                // 1 past the current test buffer
  Byte *const end = b->data + b->len;   // 1 past the end of the input buffer

  NEW_BUFFER( b );
  b->len = 0;

  // loop, absorbing one more char from the input buffer on each pass
  while ( head != end ) {
    // establish where the scan can begin
    if ( head - p_prev > (( 1 << DISP_BITS )-1) )
      p_prev = head - (( 1 << DISP_BITS )-1);

    // scan in the previous data for a match
    p = mem_find( p_prev, tail - p_prev, head, tail - head );

    // on a mismatch or end of buffer, issued codes
    if ( !p || p == head || tail - head > (1 << COUNT_BITS) + 2 ||
         tail == end ) {
      // issued the codes
      // first, check for short runs
      if ( tail - head < 4 )
        put_byte( b, *head++, &space );
      else {
        unsigned dist = head - p_prev;
        unsigned compound = (dist << COUNT_BITS) + tail - head - 4;

        if ( dist >= ( 1 << DISP_BITS ) || tail - head - 4 > 7 )
          PRINT_ERR( "%s: error: dist overflow\n", me );

        // for longer runs, issue a run-code
        // issue space char if required
        if ( space ) {
          b->data[ b->len++ ] = ' ';
          space = false;
        }

        b->data[ b->len++ ] = 0x80 + ( compound >> 8 );
        b->data[ b->len++ ] = compound & 0xFF;
        head = tail - 1;                // and start again
      }
      p_prev = buf_orig;                // start search again
    } else
      p_prev = p;                       // got a match

    // when we get to the end of the buffer, don't inc past the
    // end; this forces the residue chars out one at a time
    if ( tail != end )
      ++tail;
  } // while
  free( buf_orig );

  if ( space )
    b->data[ b->len++ ] = ' ';          // add left-over space

  size_t i, j;

  // final scan to merge consecutive high chars together
  for ( i = j = 0; i < b->len; ++i, ++j ) {
    b->data[ j ] = b->data[ i ];

    // skip run-length codes
    if ( b->data[ j ] >= 0x80 && b->data[ j ] < 0xC0 )
      b->data[ ++j ] = b->data[ ++i ];

    // if we hit a high char marker, look ahead for another
    else if ( b->data[ j ] == '\1' ) {
      b->data[ j + 1 ] = b->data[ i + 1 ];
      while ( i + 2 < b->len && b->data[ i + 2 ] == 1 && b->data[ j ] < 8 ) {
        b->data[ j ]++;
        b->data[ j + b->data[ j ] ] = b->data[ i + 3 ];
        i += 2;
      } // while
      j += b->data[ j ];
      ++i;
    }
  } // for
  b->len = j;
}

/**
 * Replaces the given buffer with an uncompressed version of itself.
 *
 * @param b The buffer to be uncompressed.
 */
void uncompress( buffer_t *b ) {
  assert( b != NULL );

  Byte *const new_data = MALLOC( Byte, BUFFER_SIZE );
  size_t i, j;

  for ( i = j = 0; i < b->len; ) {
    unsigned c = b->data[ i++ ];

    if ( c >= 1 && c <= 8 )
      while ( c-- )                     // copy 'c' bytes
        new_data[ j++ ] = b->data[ i++ ];

    else if ( c <= 0x7F )               // 0,09-7F = self
      new_data[ j++ ] = c;

    else if ( c >= 0xC0 )               // space + ASCII char
      new_data[ j++ ] = ' ', new_data[ j++ ] = c ^ 0x80;

    else {                              // 80-BF = sequences
      c = (c << 8) + b->data[ i++ ];
      int const di = (c & 0x3FFF) >> COUNT_BITS;
      for ( int n = (c & ((1 << COUNT_BITS) - 1)) + 3; n--; ++j )
        new_data[ j ] = new_data[ j - di ];
    }
  } // for
  free( b->data );
  b->data = new_data;
  b->len = j;
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
