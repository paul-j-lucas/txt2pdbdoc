/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      decode.c
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
#include "options.h"
#include "palm.h"
#include "util.h"

// standard
#include <assert.h>
#include <sys/types.h>                  /* for FreeBSD */
#include <netinet/in.h>                 /* for nthol, etc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// constants
#define COMPRESSED    2
#define UNCOMPRESSED  1

#define GET_DWord(F,N) \
  BLOCK( FREAD( (N), sizeof( DWord ), 1, (F) ); *(N) = ntohl( *(N) ); )

extern void uncompress( buffer_t* );

////////// extern functions ///////////////////////////////////////////////////

/**
 * Decodes the source Doc file to a text file.
 */
void decode( void ) {

  /********** open files, read header, ensure source is a Doc file *****/

  DatabaseHdrType header;
  FREAD( &header, DatabaseHdrSize, 1, fin );
  if ( !opt_no_check_doc && (
       strncmp( header.type,    DOC_TYPE,    sizeof header.type ) ||
       strncmp( header.creator, DOC_CREATOR, sizeof header.creator ) ) ) {
    PRINT_ERR( "%s: %s is not a Doc file\n", me, fin_path );
    exit( EXIT_NOT_DOC_FILE );
  }

  int const num_records = ntohs( header.recordList.numRecords ) - 1; /* w/o rec 0 */

  /********** read record 0 ********************************************/

  SEEK_REC( fin, 0 );
  DWord offset;
  GET_DWord( fin, &offset );            // get offset of rec 0
  FSEEK( fin, offset, SEEK_SET );

  doc_record0_t rec0;
  FREAD( &rec0, sizeof rec0, 1, fin );

  int const compression = ntohs( rec0.version );
  if ( compression != COMPRESSED && compression != UNCOMPRESSED ) {
    PRINT_ERR(
      "%s: error: unknown file compression type: %d\n",
      me, compression
    );
    exit( EXIT_UNKNOWN_COMPRESSION );
  }

  /********* read Doc file record-by-record ****************************/

  FSEEK( fin, 0, SEEK_END );
  DWord const file_size = ftell( fin );

  if ( opt_verbose )
    PRINT_ERR( "%s: decoding \"%s\":", me, header.name );

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

    if ( compression == COMPRESSED )
      uncompress( &buf );

    FWRITE( buf.data, buf.len, 1, fout );
    if ( opt_verbose )
      PRINT_ERR( " %d", num_records - rec_num );
  } // for
  if ( opt_verbose )
    putc( '\n', stderr );

  fclose( fin );
  fclose( fout );
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
