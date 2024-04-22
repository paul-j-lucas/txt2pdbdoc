/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      palm.h
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

#ifndef txt2pdbdoc_palm_H
#define txt2pdbdoc_palm_H

// local
#include "pjl_config.h"
#include "unicode.h"
#include "util.h"

// standard
#include <stdint.h>
#ifdef HAVE_TIME_H
#include <time.h>                       /* for time() */
#endif /* HAVE_TIME_H */

#ifndef TXT2PDBDOC_PALM_INLINE
# define TXT2PDBDOC_PALM_INLINE _GL_INLINE
#endif /* TXT2PDBDOC_PALM_INLINE */

////////// PalmOS types ///////////////////////////////////////////////////////

typedef uint8_t   Byte;
typedef uint16_t  Word;
typedef uint32_t  DWord;

#define dmDBNameLength  32              /**< 31 chars + 1 null terminator. */
#define RECORD_SIZE_MAX 4096            /**< Palm has a max. 4K record size. */

/**
 * Every record has one of these headers.
 *
 * @sa Christopher Bey and Kathleen Dupre.  _Palm File Format Specification_,
 * Document Number 3008-003, Palm, Inc., May 16, 2000.
 */
struct RecordEntryType {
  DWord offset;                         ///< Offset to where record starts.
  struct {
    unsigned delete   : 1;
    unsigned dirty    : 1;
    unsigned busy     : 1;
    unsigned secret   : 1;
    unsigned category : 4;
  } attributes;
  Byte uniqueID[3];
};
typedef struct RecordEntryType RecordEntryType;

// Some compilers pad structures out to DWord boundaries so using sizeof()
// doesn't give the right result.
#define RecordEntrySize   8

/**
 * This is a PDB database header as currently defined by Palm, Inc.
 *
 * @sa Christopher Bey and Kathleen Dupre.  _Palm File Format Specification_,
 * Document Number 3008-003, Palm, Inc., May 16, 2000.
 */
struct RecordListType {                 // 6 bytes total
  DWord nextRecordListID;
  Word  numRecords;
};
typedef struct RecordListType RecordListType;

#define RecordListSize    6

/**
 * This is a PDB database header as currently defined by Palm, Inc.
 */
struct DatabaseHdrType {                // 78 bytes total
  char            name[ dmDBNameLength ];
  Word            attributes;
  Word            version;
  DWord           creationDate;
  DWord           modificationDate;
  DWord           lastBackupDate;
  DWord           modificationNumber;
  DWord           appInfoID;
  DWord           sortInfoID;
  char            type[4];
  char            creator[4];
  DWord           uniqueIDSeed;
  RecordListType  recordList;
};
typedef struct DatabaseHdrType DatabaseHdrType;

#define DatabaseHdrSize   78

#define SEEK_REC(F,I) \
  FSEEK_FN( (F), DatabaseHdrSize + RecordEntrySize * (I), SEEK_SET )

////////// PalmOS utility functions ///////////////////////////////////////////

#ifdef HAVE_TIME_H
# define palm_date()      ( (DWord)(time(0) + 2082844800l) )
#else
# define palm_date()      0
#endif /* HAVE_TIME_H */

/**
 * Gets the full name of the given PalmOS character
 * (useful for error messages).
 *
 * @param c The PalmOS character to get the name of.
 * @return Returns said name.
 */
NODISCARD TXT2PDBDOC_PALM_INLINE
char const* palm_to_string( Byte c ) {
  extern char const *const PALM_TO_STRING_TABLE[];
  return PALM_TO_STRING_TABLE[ c ];
}

/**
 * Maps a PalmOS character into its corresponding Unicode codepoint.
 *
 * @param c The PalmOS character to map.
 * @return Returns said codepoint or 0 if the PalmOS character can not be
 * mapped into Unicode.
 *
 * @sa unicode_to_palm()
 */
NODISCARD TXT2PDBDOC_PALM_INLINE
char32_t palm_to_unicode( Byte c ) {
  extern char32_t const PALM_TO_UNICODE_TABLE[];
  return PALM_TO_UNICODE_TABLE[ c ];
}

/**
 * Maps a Unicode codepoint into its corresponding PalmOS character.
 *
 * @param codepoint The Unicode codepoint to map.
 * @return Returns said character or 0 if the codepoint can not be mapped into
 * a PalmOS character.
 *
 * @sa palm_to_unicode()
 */
NODISCARD
Byte unicode_to_palm( char32_t codepoint );

///////////////////////////////////////////////////////////////////////////////

#endif /* txt2pdbdoc_palm_H */
/* vim:set et sw=2 ts=2: */
