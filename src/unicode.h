/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      unicode.h
**
**      Copyright (C) 2015-2024  Paul J. Lucas
**
**      This program is free software; you can redistribute it and/or modify
**      it under the terms of the GNU General Public License as published by
**      the Free Software Foundation; either version 2 of the Licence, or
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

#ifndef txt2pdbdoc_utf8_H
#define txt2pdbdoc_utf8_H

// local
#include "pjl_config.h"

// standard
#include <stdbool.h>
#include <stdint.h>                     /* for uint8_t, ... */

#if !HAVE_CHAR8_T
typedef uint8_t char8_t;                /* borrowed from C++20 */
#endif /* !HAVE_CHAR8_T */
#if !HAVE_CHAR16_T
typedef uint16_t char16_t;              /* C11's char16_t */
#endif /* !HAVE_CHAR16_T */
#if !HAVE_CHAR32_T
typedef uint32_t char32_t;              /* C11's char32_t */
#endif /* !HAVE_CHAR32_T */

#ifndef TXT2PDBDOC_UNICODE_INLINE
# define TXT2PDBDOC_UNICODE_INLINE _GL_INLINE
#endif /* TXT2PDBDOC_UNICODE_INLINE */

///////////////////////////////////////////////////////////////////////////////

#define CP_INVALID                0x01FFFFu /**< Invalid Unicode code-point. */
#define CP_SURROGATE_HIGH_START   0x00D800u /**< Unicode surrogate high. */
#define CP_SURROGATE_LOW_END      0x00DFFFu /**< Unicode surrogate low. */
#define CP_VALID_MAX              0x10FFFFu /**< Maximum valid code-point. */
#define UTF8_CHAR_SIZE_MAX        4     /**< Bytes needed for UTF-8 char. */

///////////////////////////////////////////////////////////////////////////////

/**
 * Checks whether \a cp is an ASCII character.
 *
 * @param cp The Unicode code-point to check.
 * @return Returns `true` only if \a cp is an ASCII character.
 */
NODISCARD TXT2PDBDOC_UNICODE_INLINE
bool cp_is_ascii( char32_t cp ) {
  return cp <= 0x7F;
}

/**
 * Checks whether the given Unicode code-point is valid.
 *
 * @param cp_candidate The Unicode code-point candidate value to check.
 * @return Returns `true` only if \a cp_candidate is a valid code-point.
 */
NODISCARD TXT2PDBDOC_UNICODE_INLINE
bool cp_is_valid( uint64_t cp_candidate ) {
  return   cp_candidate < CP_SURROGATE_HIGH_START
      ||  (cp_candidate > CP_SURROGATE_LOW_END && cp_candidate <= 0x00FFFD)
      ||  (cp_candidate >= 0x010000 && cp_candidate <= CP_VALID_MAX);
}

/**
 * Parses a Unicode code-point value.
 *
 * @param s The NULL-terminated string to parse.  Allows for strings of the
 * form:
 *  + X: a single character.
 *  + NN: two-or-more decimal digits.
 *  + 0xN, u+N, or U+N: one-or-more hexadecimal digits.
 * @return Returns the Unicode code-point value
 * or prints an error message and exits if \a s is invalid.
 */
NODISCARD
char32_t parse_codepoint( char const *s );

/**
 * Gets the length of a UTF-8 character.
 *
 * @param start The start byte of a UTF-8 byte sequence.
 * @return Returns the number of bytes needed for the UTF-8 character in the
 * range [1,6] or 0 if \a start is not a valid start byte.
 */
NODISCARD TXT2PDBDOC_UNICODE_INLINE
unsigned utf8_char_len( char8_t start ) {
  extern char8_t const UTF8_LEN_TABLE[];
  return UTF8_LEN_TABLE[ start ];
}

/**
 * Decodes a UTF-8 octet sequence into a Unicode codepoint.
 *
 * @param utf8 The UTF-8 octet sequence to decode.
 * @return Returns said codepoint or 0 if the \a utf8 is invalid.
 */
NODISCARD
char32_t utf8_decode( char8_t const *utf8 );

/**
 * Encodes a Unicode codepoint into UTF-8.
 *
 * @param cp The Unicode code-point to encode.
 * @param utf8_buf A pointer to the start of a buffer to receive the UTF-8
 * bytes; must be at least #UTF8_CHAR_SIZE_MAX long.  No NULL byte is appended.
 * @return Returns the number of bytes comprising the codepoint encoded as
 * UTF-8.
 */
NODISCARD
unsigned utf8_encode( char32_t cp, char8_t *utf8_buf );

///////////////////////////////////////////////////////////////////////////////

_GL_INLINE_HEADER_END

#endif /* txt2pdbdoc_utf8_H */
/* vim:set et sw=2 ts=2: */
