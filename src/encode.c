/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      encode.c
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
#include "common.h"
#include "doc.h"
#include "options.h"
#include "palm.h"
#include "utf8.h"
#include "util.h"

// standard
#include <assert.h>
#include <ctype.h>
#include <sys/types.h>                  /* for FreeBSD */
#include <netinet/in.h>                 /* for htonl() */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define PUT_DWord(F,N) \
  BLOCK( DWord t = (N); t = htonl(t); FWRITE( &t, sizeof t, 1, (F) ); )

////////// extern declarations ////////////////////////////////////////////////

extern char const  *doc_name;
extern FILE        *fin;
extern FILE        *fout;
extern char const  *fout_path;

extern bool         opt_binary;
extern bool         opt_compress;
extern bool         opt_no_timestamp;
extern bool         opt_no_warnings;
extern bool         opt_verbose;

extern void compress( buffer_t* );

////////// local functions ////////////////////////////////////////////////////

/**
 * Fills a buffer with characters from the input file.
 *
 * @param buf The buffer to fill.
 */
static void fill_buffer( buffer_t *buf ) {
  assert( buf != NULL );
  assert( buf->data != NULL );
  buf->len = 0;

  for ( int c; (c = getc( fin )) != EOF; ) {
    size_t const len = utf8_len( c );
    if ( len == 0 ) {
      if ( !opt_no_warnings )
        PMESSAGE( "\"%s\": invalid UTF-8 start byte\n", printable_char( c ) );
      continue;
    }

    ////////// handle ASCII ///////////////////////////////////////////////////

    if ( len == 1 ) {
      if ( opt_binary ) {
        if ( !(isspace( c ) || isprint( c )) )
          continue;
        switch ( c ) {
          case '\r':
            if ( peekc( fin ) == '\n' )
              continue;                     // CR+LF -> LF
            // no break;
          case '\f':
            c = '\n';
            break;
        } // switch
      }
      buf->data[ buf->len ] = (Byte)c;
    }

    ////////// handle UTF-8 ///////////////////////////////////////////////////

    else {
      uint8_t utf8_char[ UTF8_LEN_MAX ];
      size_t u = 0;
      utf8_char[ u++ ] = c;
      for ( size_t i = len; i > 1; --i ) {
        if ( (c = getc( fin )) == EOF )
          goto done;
        if ( utf8_len( c ) ) {
          if ( !opt_no_warnings ) {
            PMESSAGE(
              "\"%s\": invalid UTF-8 continuation byte\n",
              printable_char( c )
            );
          }
          goto next;
        }
        utf8_char[ u++ ] = c;
      } // for
      uint32_t const codepoint = utf8_decode( utf8_char );
      if ( !(c = unicode_to_palm( codepoint )) ) {
        if ( !opt_no_warnings ) {
          PMESSAGE(
            "\"%x04X\": Unicode codepoint does not map to PalmOS\n", codepoint
          );
        }
        continue;
      }
    }

    buf->data[ buf->len ] = c;
    if ( ++buf->len == RECORD_SIZE_MAX )
      break;

next:
    /* nothing */;
  } // for

done:
  if ( ferror( fin ) )
    PERROR_EXIT( READ_ERROR );
}

////////// extern functions ///////////////////////////////////////////////////

/**
 * Encodes the source text file into a Doc file.
 */
void encode( void ) {
  struct stat sbuf;
  FSTAT( fileno( fin ), &sbuf );
  DWord const fin_size = sbuf.st_size;

  int num_records = fin_size / RECORD_SIZE_MAX;
  if ( (DWord)num_records * RECORD_SIZE_MAX < fin_size )
    ++num_records;

  ////////// create and write header //////////////////////////////////////////

  DatabaseHdrType header;
  memset( &header, 0, sizeof header );

  strncpy( header.name, doc_name, sizeof header.name - 1 );
  if ( strlen( doc_name ) > sizeof header.name - 1 )
    strncpy( header.name + sizeof header.name - 4, "...", 3 );

  if ( !opt_no_timestamp ) {
    DWord const now = htonl( palm_date() );
    header.creationDate                 = now;
    header.modificationDate             = now;
  }
  strncpy( header.type,    DOC_TYPE,    sizeof header.type );
  strncpy( header.creator, DOC_CREATOR, sizeof header.creator );
  header.recordList.numRecords          = htons( num_records + 1 /* rec 0 */ );

  FWRITE( &header, DatabaseHdrSize, 1, fout );

  ////////// write record offsets /////////////////////////////////////////////

  DWord num_offsets = num_records + 1;  // +1 for rec 0
  DWord offset = DatabaseHdrSize + RecordEntrySize * num_offsets;
  DWord index = 0x40u << 24 | 0x6F8000u; // dirty + unique ID

  PUT_DWord( fout, offset );            // offset for rec 0
  PUT_DWord( fout, index++ );

  while( --num_offsets ) {
    PUT_DWord( fout, 0 );               // placeholder
    PUT_DWord( fout, index++ );
  }

  ////////// write record 0 ///////////////////////////////////////////////////

  doc_record0_t rec0;
  memset( &rec0, 0, sizeof rec0 );

  rec0.version     = htons( opt_compress + 1 );
  rec0.doc_size    = htonl( fin_size );
  rec0.num_records = htons( num_records );
  rec0.rec_size    = htons( RECORD_SIZE_MAX );

  FWRITE( &rec0, sizeof rec0, 1, fout );

  ////////// write text ///////////////////////////////////////////////////////

  buffer_t buf;
  NEW_BUFFER( &buf );
  int total_before = 0, total_after = 0;

  for ( int rec_num = 1; rec_num <= num_records; ++rec_num ) {
    offset = ftell( fout );
    SEEK_REC( fout, rec_num );
    PUT_DWord( fout, offset );

    fill_buffer( &buf );
    size_t const uncompressed_buf_len = buf.len;
    if ( opt_compress )
      compress( &buf );

    FSEEK( fout, offset, SEEK_SET );
    FWRITE( buf.data, buf.len, 1, fout );

    if ( !opt_verbose )
      continue;

    if ( opt_compress ) {
      PRINT_ERR(
        "  record %2d: %5zu bytes -> %5zu (%2d%%)\n",
        rec_num, uncompressed_buf_len, buf.len,
        (int)( 100.0 * buf.len / uncompressed_buf_len )
      );
      total_before += uncompressed_buf_len;
      total_after  += buf.len;
    } else {
      PRINT_ERR( " %d", num_records - rec_num + 1 );
    }
  } // for

  if ( opt_verbose ) {
    if ( opt_compress ) {
      PRINT_ERR( "\n-----\ntotal compression: %2d%%\n",
        (int)( 100.0 * total_after / total_before )
      );
    } else {
      FPUTC( '\n', stderr );
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
