/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      utf8.c
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
#include "utf8.h"

// standard
#include <assert.h>
#include <ctype.h>

///////////////////////////////////////////////////////////////////////////////

uint8_t const utf8_len_table[] = {
  /*      0 1 2 3 4 5 6 7 8 9 A B C D E F */
  /* 0 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 1 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 2 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 3 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 4 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 5 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 6 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 7 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 8 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // continuation bytes
  /* 9 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  //        |
  /* A */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  //        |
  /* B */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  //        |
  /* C */ 0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  // C0 & C1 are overlong ASCII
  /* D */ 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  /* E */ 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
  /* F */ 4,4,4,4,4,4,4,4,5,5,5,5,6,6,0,0
};

////////// extern functions ///////////////////////////////////////////////////

uint32_t utf8_decode( uint8_t const *u ) {
  assert( u );

  size_t const len = utf8_len( *u );
  if ( len == 1 )                       // special-case ASCII
    return *u & 0xFFu;                  // prevents sign-extension

  uint32_t codepoint = 0;
  unsigned m = (0x7F >> len) & 0x1F;    // mask
  switch ( len ) {
    case 6: codepoint |= ((*u & m   ) << 30); ++u; m = 0x3F;
    case 5: codepoint |= ((*u & m   ) << 24); ++u; m = 0x3F;
    case 4: codepoint |= ((*u & m   ) << 18); ++u; m = 0x3F;
    case 3: codepoint |= ((*u & m   ) << 12); ++u; m = 0x3F;
    case 2: codepoint |= ((*u & m   ) <<  6); ++u;
            codepoint |=  (*u & 0x3F)       ; ++u;
            break;
  } // switch
  return codepoint;
}

size_t utf8_encode( uint32_t codepoint, uint8_t *u ) {
  assert( u );

  static unsigned const Mask1 = 0x80;
  static unsigned const Mask2 = 0xC0;
  static unsigned const Mask3 = 0xE0;
  static unsigned const Mask4 = 0xF0;
  static unsigned const Mask5 = 0xF8;
  static unsigned const Mask6 = 0xFC;

  uint32_t const n = codepoint & 0xFFFFFFFF;
  uint8_t *const u0 = u;
  if ( isascii( n ) ) {
    // 0xxxxxxx
    *u++ = (uint8_t)n;
  } else if ( n < 0x800 ) {
    // 110xxxxx 10xxxxxx
    *u++ = (uint8_t)( Mask2 |  (n >>  6)         );
    *u++ = (uint8_t)( Mask1 | ( n        & 0x3F) );
  } else if ( n < 0x10000 ) {
    // 1110xxxx 10xxxxxx 10xxxxxx
    *u++ = (uint8_t)( Mask3 |  (n >> 12)         );
    *u++ = (uint8_t)( Mask1 | ((n >>  6) & 0x3F) );
    *u++ = (uint8_t)( Mask1 | ( n        & 0x3F) );
  } else if ( n < 0x200000 ) {
    // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    *u++ = (uint8_t)( Mask4 |  (n >> 18)         );
    *u++ = (uint8_t)( Mask1 | ((n >> 12) & 0x3F) );
    *u++ = (uint8_t)( Mask1 | ((n >>  6) & 0x3F) );
    *u++ = (uint8_t)( Mask1 | ( n        & 0x3F) );
  } else if ( n < 0x4000000 ) {
    // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    *u++ = (uint8_t)( Mask5 |  (n >> 24)         );
    *u++ = (uint8_t)( Mask1 | ((n >> 18) & 0x3F) );
    *u++ = (uint8_t)( Mask1 | ((n >> 12) & 0x3F) );
    *u++ = (uint8_t)( Mask1 | ((n >>  6) & 0x3F) );
    *u++ = (uint8_t)( Mask1 | ( n        & 0x3F) );
  } else if ( n < 0x8000000 ) {
    // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    *u++ = (uint8_t)( Mask6 |  (n >> 30)         );
    *u++ = (uint8_t)( Mask1 | ((n >> 24) & 0x3F) );
    *u++ = (uint8_t)( Mask1 | ((n >> 18) & 0x3F) );
    *u++ = (uint8_t)( Mask1 | ((n >> 12) & 0x3F) );
    *u++ = (uint8_t)( Mask1 | ((n >>  6) & 0x3F) );
    *u++ = (uint8_t)( Mask1 | ( n        & 0x3F) );
  }
  return u - u0;
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
