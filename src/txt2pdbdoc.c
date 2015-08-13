/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      txt2pdbdoc.c
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
#include "palm.h"
#include "util.h"

// standard
#include <assert.h>
#include <sys/types.h>                  /* for FreeBSD */
#include <netinet/in.h>                 /* for htonl, etc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// constants
#define COMPRESSED    2
#define DOC_CREATOR   "REAd"
#define DOC_TYPE      "TEXt"
#define UNCOMPRESSED  1

// macros

#define GET_Word(F,N) \
  BLOCK( FREAD( &N, 2, 1, (F) ); N = ntohs(N); )

#define GET_DWord(F,N) \
  BLOCK( FREAD( &N, 4, 1, (F) ); N = ntohl(N); )

#define PUT_Word(F,N) \
  BLOCK( Word const temp = htons(N); FWRITE( &temp, 2, 1, (F) ); )

#define PUT_DWord(F,N) \
  BLOCK( DWord const temp = htonl(N); FWRITE( &temp, 4, 1, (F) ); )

#define SEEK_REC_ENTRY(F,I) \
  FSEEK_FN( (F), DatabaseHdrSize + RecordEntrySize * (I), SEEK_SET )

/**
 * Record 0 of a Doc file contains information about the document as a whole.
 */
struct doc_record0 {                    // 16 bytes total
  Word  version;                        // 1 = plain text, 2 = compressed
  Word  reserved1;
  DWord doc_size;                       // in bytes, when uncompressed
  Word  num_records;                    // PDB header numRecords - 1
  Word  rec_size;                       // usually RECORD_SIZE_MAX
  DWord reserved2;
};
typedef struct doc_record0 doc_record0_t;

extern void compress( buffer_t* );
extern void uncompress( buffer_t* );

/*****************************************************************************
 *
 *  Globals
 *
 *****************************************************************************/

char const* me;                         // executable name

bool    opt_binary = true;
bool    opt_compress = true;
bool    opt_no_check_doc = false;
bool    opt_verbose = false;

void    decode( char const*, char const* );
void    encode( char const*, char const*, char const* );
void    remove_binary( buffer_t* );
static void usage( void );

int main( int argc, char *argv[] ) {
  bool        decode_opt = false;
  char const  opts[] = "bcdDvV";        // command line options
  int         opt;                      // option being processed

  me = strrchr( argv[0], '/' );         // determine base name...
  me = me ? me + 1 : argv[0];           // ...of executable

  /********** Process command-line options *****************************/

  opterr = 1;
  while ( (opt = getopt( argc, argv, opts )) != EOF ) {
    switch ( opt ) {
      case 'b': opt_binary = false;                 break;
      case 'c': opt_compress = false;               break;
      case 'd': decode_opt = true;                  break;
      case 'D': opt_no_check_doc = true;            break;
      case 'v': opt_verbose = true;                 break;
      case 'V': printf( PACKAGE " " VERSION "\n" ); exit( EXIT_SUCCESS );
      default : usage();
    } // switch
  } // while
  argc -= optind, argv += optind;

  if ( decode_opt ) {
    if ( argc < 1 || argc > 2 ) usage();
    decode( argv[0], argc == 2 ? argv[1] : 0 );
  } else {
    if ( argc != 3 ) usage();
    encode( argv[0], argv[1], argv[2] );
  }

  exit( EXIT_SUCCESS );
}

/**
 * Decodes the source Doc file to a text file.
 *
 * @param src_file_name The name of the Doc file.
 * @param dest_file_name The name of the text file.  If NULL, text is sent to
 * standard output.
 */
void decode( char const *src_file_name, char const *dest_file_name ) {

  /********** open files, read header, ensure source is a Doc file *****/

  FILE *const fin = check_fopen( src_file_name, "rb" );

  DatabaseHdrType header;
  FREAD( &header, DatabaseHdrSize, 1, fin );
  if ( !opt_no_check_doc && (
       strncmp( header.type,    DOC_TYPE,    sizeof header.type ) ||
       strncmp( header.creator, DOC_CREATOR, sizeof header.creator )
  ) ) {
    PRINT_ERR( "%s: %s is not a Doc file\n", me, src_file_name );
    exit( EXIT_NOT_DOC_FILE );
  }

  int const num_records = ntohs( header.recordList.numRecords ) - 1; /* w/o rec 0 */

  FILE *const fout = dest_file_name ?
    check_fopen( dest_file_name, "wb" ) : stdout;

  /********** read record 0 ********************************************/

  SEEK_REC_ENTRY( fin, 0 );
  DWord offset;
  GET_DWord( fin, offset );             // get offset of rec 0
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
    DWord next_offset;

    /* read the record offset */
    SEEK_REC_ENTRY( fin, rec_num );
    GET_DWord( fin, offset );

    // read the next record offset to compute the record size
    if ( rec_num < num_records ) {
      SEEK_REC_ENTRY( fin, rec_num + 1 );
      GET_DWord( fin, next_offset );
    } else
      next_offset = file_size;
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

/**
 * Encodes the source text file into a Doc file.
 *
 * @param document_name The name of the document as it is to appear in the
 * Documents List view of a Doc reader application on the Pilot.
 * @param src_file_name The name of the text file.
 * @param dest_file_name  The name of the Doc file.
 */
void encode( char const *document_name, char const *src_file_name,
             char const *dest_file_name ) {
  assert( document_name );
  assert( src_file_name );
  assert( dest_file_name );

  DWord         date;
  doc_record0_t   rec0;
  buffer_t        buf;
  DWord         num_offsets, offset;
  unsigned long index;
  int           total_before, total_after;

  FILE *const fin  = check_fopen( src_file_name, "rb" );
  FILE *const fout = check_fopen( dest_file_name, "wb" );

  FSEEK( fin, 0, SEEK_END );
  DWord file_size = ftell( fin );
  int num_records = file_size / RECORD_SIZE_MAX;
  if ( (long)num_records * RECORD_SIZE_MAX < file_size )
    ++num_records;

  ////////// create and write header //////////////////////////////////////////

  DatabaseHdrType header;
  bzero( header.name, sizeof header.name );
  strncpy( header.name, document_name, sizeof header.name - 1 );
  if ( strlen( document_name ) > sizeof header.name - 1 )
    strncpy( header.name + sizeof header.name - 4, "...", 3 );
  header.attributes                       = 0;
  header.version                          = 0;
  date = htonl( palm_date() );
  memcpy( &header.creationDate,   &date, 4 );
  date = htonl( palm_date() );
  memcpy( &header.modificationDate, &date, 4 );
  header.lastBackupDate                 = 0;
  header.modificationNumber             = 0;
  header.appInfoID                      = 0;
  header.sortInfoID                     = 0;
  strncpy( header.type,    DOC_TYPE,    sizeof header.type );
  strncpy( header.creator, DOC_CREATOR, sizeof header.creator );
  header.uniqueIDSeed                 = 0;
  header.recordList.nextRecordListID  = 0;
  header.recordList.numRecords        = htons( num_records + 1 /* rec 0 */ );
  FSEEK( fin, 0, SEEK_SET );
  FWRITE( &header, DatabaseHdrSize, 1, fout );

  /********** write record offsets *************************************/

  num_offsets = num_records + 1;        // +1 for rec 0
  offset = DatabaseHdrSize + RecordEntrySize * num_offsets;
  index = 0x40 << 24 | 0x6F8000;        // dirty + unique ID

  PUT_DWord( fout, offset );            // offset for rec 0
  PUT_DWord( fout, index++ );

  while( --num_offsets ) {
    PUT_DWord( fout, 0 );               // placeholder
    PUT_DWord( fout, index++ );
  }

  /********** write record 0 *******************************************/

  rec0.version     = htons( opt_compress + 1 );
  rec0.reserved1   = 0;
  rec0.doc_size    = htonl( file_size );
  rec0.num_records = htons( num_records );
  rec0.rec_size    = htons( RECORD_SIZE_MAX );
  rec0.reserved2   = 0;

  FWRITE( &rec0, sizeof rec0, 1, fout );

  /********** write text ***********************************************/

  NEW_BUFFER( &buf );
  total_before = total_after = 0;
  for ( int rec_num = 1; rec_num <= num_records; ++rec_num ) {
    offset = ftell( fout );
    SEEK_REC_ENTRY( fout, rec_num );
    PUT_DWord( fout, offset );

    size_t bytes_read;
    if ( !(bytes_read = fread( buf.data, RECORD_SIZE_MAX, 1, fin )) )
      break;
    if ( ferror( fin ) )
      PERROR_EXIT( READ_ERROR );
    buf.len = bytes_read;

    if ( opt_binary )
      remove_binary( &buf );
    if ( opt_compress )
      compress( &buf );

    FSEEK( fout, offset, SEEK_SET );
    FWRITE( buf.data, buf.len, 1, fout );

    if ( !opt_verbose )
      continue;

    if ( opt_compress ) {
      PRINT_ERR(
        "  record %2d: %5zu bytes -> %5zu (%2d%%)\n",
        rec_num, bytes_read, buf.len,
        (int)( 100.0 * buf.len / bytes_read )
      );
      total_before += bytes_read;
      total_after  += buf.len;
    } else
      PRINT_ERR( " %d", num_records - rec_num + 1 );
  } // for
  if ( opt_verbose ) {
    if ( opt_compress )
      PRINT_ERR( "\n-----\ntotal compression: %2d%%\n",
        (int)( 100.0 * total_after / total_before )
      );
    else
      putc( '\n', stderr );
  }

  fclose( fin );
  fclose( fout );
}

/**
 * Replaces the given buffer with one that has had ASCII characters (0-8)
 * removed and carriage-returns and form-feeds converted to newlines.
 *
 * @param b The buffer to be affected.
 */
void remove_binary( buffer_t *b ) {
  assert( b );

  Byte *const new_data = MALLOC( Byte, b->len );
  size_t i, j;

  for ( i = j = 0; i < b->len; ++i ) {
    if ( b->data[ i ] < 9 )             // discard really low ASCII
      continue;
    switch ( b->data[ i ] ) {
      case '\r':
        if ( i < b->len - 1 && b->data[ i+1 ] == '\n' )
          continue;                     // CR+LF -> LF
        // no break;
      case '\f':
        new_data[ j ] = '\n';
        break;
      default:
        new_data[ j ] = b->data[ i ];
    } // switch
    ++j;
  } // for
  free( b->data );
  b->data = new_data;
  b->len = j;
}

/*****************************************************************************
 *
 *  Miscellaneous function(s)
 *
 *****************************************************************************/

static void usage( void ) {
  PRINT_ERR(
"usage: %s [-c] [-b] [-v] document_name file.txt file.pdb\n"
"       %s  -d  [-D] [-v] file.pdb [file.txt]\n"
"       %s  -V\n"
"\n"
"options:\n"
" -b: Don't strip binary characters [default: do].\n"
" -c: Don't compress Doc file [default: do].\n"
" -d: Decode Doc file to text [default: encode to Doc].\n"
" -D: Do not check the type/creator of the Doc file [default: do].\n"
" -v: Be verbose [default: don't].\n"
" -V: Print version and exit.\n"
    , me, me, me
  );
  exit( EXIT_USAGE );
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
