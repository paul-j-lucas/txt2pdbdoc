/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      decode.c
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
#include "common.h"
#include "doc.h"
#include "options.h"
#include "palm.h"
#include "unicode.h"
#include "util.h"

// standard
#include <assert.h>
#include <ctype.h>
#include <sys/types.h>                  /* for FreeBSD */
#include <netinet/in.h>                 /* for nthol, etc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define GET_DWord(F,N) \
  BLOCK( FREAD( (N), sizeof( DWord ), 1, (F) ); *(N) = ntohl( *(N) ); )

////////// extern declarations ////////////////////////////////////////////////

extern FILE        *fin;
extern char const  *fin_path;
extern FILE        *fout;

extern bool         opt_no_check_doc;
extern bool         opt_no_warnings;
extern char32_t     opt_unmapped_codepoint;
extern bool         opt_verbose;

extern void uncompress( buffer_t* );

////////// local functions ////////////////////////////////////////////////////

/**
 * Maps a PalmOS character into its corresponding UTF-8 octet sequence.
 *
 * @param c The PalmOS character to map.
 * @return Returns said sequence or NULL if the PalmOS character can not be
 * mapped into Unicode.
 */
NODISCARD
static char8_t const* palm_to_utf8( Byte c ) {
  char32_t cp = palm_to_unicode( c );

  if ( cp == 0 ) {
    if ( !opt_no_warnings )
      PMESSAGE(
        "\"%s\" (%s): PalmOS character does not map to Unicode%s\n",
        printable_char( STATIC_CAST( char, c ) ), palm_to_string( c ),
        (opt_unmapped_codepoint ? "" : ": skipped")
      );
    if ( !opt_unmapped_codepoint )
      return NULL;
    cp = opt_unmapped_codepoint;
  }

  switch ( cp ) {
    case 0x81:
    case 0x9B:
      //
      // These characters are not used in PalmOS and shouldn't be present;
      // but since we got them, skip them.
      //
      if ( !opt_no_warnings )
        PMESSAGE(
          "\"%s\": character unused by PalmOS: skipped\n",
          printable_char( STATIC_CAST( char, c ) )
        );
      return NULL;
  } // switch

  static char8_t utf8_char[ UTF8_CHAR_SIZE_MAX + 1 /*NULL*/ ];
  unsigned len;

  if ( cp_is_ascii( cp ) ) {
    if ( !(isspace( (int)cp ) || isprint( (int)cp )) ) {
      PMESSAGE(
        "\"%s\" (%s): non-printable character found: skipped\n",
        printable_char( STATIC_CAST( char, c ) ), palm_to_string( c )
      );
      return NULL;
    }
    utf8_char[0] = c;
    len = 1;
  } else {
    len = utf8_encode( cp, utf8_char );
  }

  utf8_char[ len ] = '\0';
  return utf8_char;
}

////////// extern functions ///////////////////////////////////////////////////

/**
 * Decodes the source Doc file to a text file.
 */
void decode( void ) {

  ////////// open files, read header, ensure source is a Doc file /////////////

  DatabaseHdrType header;
  FREAD( &header, DatabaseHdrSize, 1, fin );
  if ( !opt_no_check_doc && (
       strncmp( header.type,    DOC_TYPE,    sizeof header.type ) ||
       strncmp( header.creator, DOC_CREATOR, sizeof header.creator ) ) ) {
    PMESSAGE_EXIT( EX_DATAERR, "%s is not a Doc file\n", fin_path );
  }

  // without rec 0
  int const num_records = ntohs( header.recordList.numRecords ) - 1;

  ////////// read record 0 ////////////////////////////////////////////////////

  SEEK_REC( fin, 0 );
  DWord offset;
  GET_DWord( fin, &offset );            // get offset of rec 0
  FSEEK( fin, offset, SEEK_SET );

  doc_record0_t rec0;
  FREAD( &rec0, sizeof rec0, 1, fin );

  int const compression = ntohs( rec0.version );
  switch ( compression ) {
    case DOC_COMPRESSED:
    case DOC_UNCOMPRESSED:
      break;
    default:
      PMESSAGE_EXIT( EX_DATAERR,
        "error: %d: unknown file compression type\n", compression
      );
  } // switch

  ///////// read Doc file record-by-record ////////////////////////////////////

  FSEEK( fin, 0, SEEK_END );
  DWord const file_size = STATIC_CAST( DWord, ftell( fin ) );

  if ( opt_verbose )
    PMESSAGE( "decoding \"%s\":", header.name );

  buffer_t buf;
  NEW_BUFFER( &buf );
  for ( int rec_num = 1; rec_num <= num_records; ++rec_num ) {

    // read the record offset
    SEEK_REC( fin, rec_num );
    GET_DWord( fin, &offset );

    // read the next record offset to compute the record size
    DWord next_offset;
    if ( rec_num < num_records ) {
      SEEK_REC( fin, rec_num + 1 );
      GET_DWord( fin, &next_offset );
    } else {
      next_offset = file_size;
    }
    DWord const rec_size = next_offset - offset;

    // read the record
    FSEEK( fin, offset, SEEK_SET );
    FREAD( buf.data, 1, rec_size, fin );
    buf.len = rec_size;

    if ( compression == DOC_COMPRESSED )
      uncompress( &buf );

    for ( size_t i = 0; i < buf.len; ++i ) {
      char8_t const *const utf8_char = palm_to_utf8( buf.data[i] );
      if ( utf8_char )
        FPRINTF( fout, "%s", utf8_char );
    } // for

    if ( opt_verbose )
      PRINT_ERR( " %d", num_records - rec_num );
  } // for

  if ( opt_verbose )
    FPUTC( '\n', stderr );
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
