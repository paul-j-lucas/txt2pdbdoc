/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      unicode.c
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

// local
#include "pjl_config.h"
#define TXT2PDBDOC_UNICODE_INLINE _GL_EXTERN_INLINE
#include "unicode.h"
#include "util.h"

// standard
#include <assert.h>
#include <ctype.h>
#include <sysexits.h>

///////////////////////////////////////////////////////////////////////////////

char8_t const UTF8_LEN_TABLE[] = {
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

char32_t parse_codepoint( char const *s ) {
  assert( s != NULL );

  if ( s[0] != '\0' && s[1] == '\0' )   // assume single-char ASCII
    return STATIC_CAST( char32_t, s[0] );

  char const *const s0 = s;
  if ( (s[0] == 'U' || s[0] == 'u') && s[1] == '+' ) {
    // convert [uU]+NNNN to 0xNNNN so strtoull() will grok it
    char *const t = free_later( check_strdup( s ) );
    s = memcpy( t, "0x", 2 );
  }
  uint64_t const cp = parse_ull( s );
  if ( cp_is_valid( cp ) )
    return STATIC_CAST( char32_t, cp );
  PMESSAGE_EXIT( EX_USAGE,
    "\"%s\": invalid Unicode code-point for -%c\n",
    s0, 'U'
  );
}

char32_t utf8_decode( char8_t const *u ) {
  assert( u != NULL );

  unsigned const len = utf8_char_len( *u );
  if ( len == 1 )                       // special-case ASCII
    return *u & 0xFFu;                  // prevents sign-extension

  char32_t cp = 0;
  unsigned m = (0x7F >> len) & 0x1F;    // mask

  switch ( len ) {
    case 6: cp |= ((*u & m   ) << 30); ++u; m = 0x3F; FALLTHROUGH;
    case 5: cp |= ((*u & m   ) << 24); ++u; m = 0x3F; FALLTHROUGH;
    case 4: cp |= ((*u & m   ) << 18); ++u; m = 0x3F; FALLTHROUGH;
    case 3: cp |= ((*u & m   ) << 12); ++u; m = 0x3F; FALLTHROUGH;
    case 2: cp |= ((*u & m   ) <<  6); ++u;
            cp |=  (*u & 0x3F)       ; ++u;
            break;
  } // switch
  return cp;
}

size_t utf8_encode( char32_t cp, char8_t *u8 ) {
  assert( u8 != NULL );

  static unsigned const Mask1 = 0x80;
  static unsigned const Mask2 = 0xC0;
  static unsigned const Mask3 = 0xE0;
  static unsigned const Mask4 = 0xF0;

  char8_t const *const u8_orig = u8;
  if ( cp < 0x80 ) {
    // 0xxxxxxx
    *u8++ = STATIC_CAST( char8_t, cp );
  }
  else if ( cp < 0x800 ) {
    // 110xxxxx 10xxxxxx
    *u8++ = STATIC_CAST( char8_t, Mask2 |  (cp >>  6)         );
    *u8++ = STATIC_CAST( char8_t, Mask1 | ( cp        & 0x3F) );
  }
  else if ( cp < 0x10000 ) {
    // 1110xxxx 10xxxxxx 10xxxxxx
    *u8++ = STATIC_CAST( char8_t, Mask3 |  (cp >> 12)         );
    *u8++ = STATIC_CAST( char8_t, Mask1 | ((cp >>  6) & 0x3F) );
    *u8++ = STATIC_CAST( char8_t, Mask1 | ( cp        & 0x3F) );
  }
  else if ( cp < 0x200000 ) {
    // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    *u8++ = STATIC_CAST( char8_t, Mask4 |  (cp >> 18)         );
    *u8++ = STATIC_CAST( char8_t, Mask1 | ((cp >> 12) & 0x3F) );
    *u8++ = STATIC_CAST( char8_t, Mask1 | ((cp >>  6) & 0x3F) );
    *u8++ = STATIC_CAST( char8_t, Mask1 | ( cp        & 0x3F) );
  }
  else {
    return STATIC_CAST( size_t, -1 );
  }

  return STATIC_CAST( size_t, u8 - u8_orig );
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
