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
#define BUFFER_SIZE   6000              /* big enough for uncompressed record */
#define COMPRESSED    2
#define COUNT_BITS    3                 /* why this value?  I don't know */
#define DISP_BITS     11                /* ditto */
#define DOC_CREATOR   "REAd"
#define DOC_TYPE      "TEXt"
#define UNCOMPRESSED  1

// macros

#define NEW_BUFFER(b) (b)->data = MALLOC( Byte, (b)->len = BUFFER_SIZE )

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

/*****************************************************************************
 *
 *  Globals
 *
 *****************************************************************************/

struct buffer {
  Byte    *data;
  unsigned len;
};
typedef struct buffer buffer_t;

char const* me;                         // executable name

bool    opt_binary = true;
bool    opt_compress = true;
bool    opt_no_check_doc = false;
bool    opt_verbose = false;

void    compress( buffer_t* );
void    decode( char const*, char const* );
void    encode( char const*, char const*, char const* );
void    put_byte( buffer_t*, Byte, bool* );
void    remove_binary( buffer_t* );
void    uncompress( buffer_t* );
static void usage();

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
 * Replaces the given buffer with a compressed version of itself.  I don't
 * understand this algorithm.  I just cleaned up the code.
 *
 * @param b The buffer to be compressed.
 */
void compress( buffer_t *b ) {
  int i, j;
  bool space = false;

  Byte *buf_orig;
  Byte *p;                  // walking test hit; works up on successive matches
  Byte *p_prev;
  Byte *head;                           // current test string

  p = p_prev = head = buf_orig = b->data;
  Byte *const tail = head + 1;          // 1 past the current test buffer
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
  }
  free( buf_orig );

  if ( space )
    b->data[ b->len++ ] = ' ';          // add left-over space

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
 * Decodes the source Doc file to a text file.
 *
 * @param src_file_name The name of the Doc file.
 * @param dest_file_name The name of the text file.  If NULL, text is sent to
 * standard output.
 */
void decode( char const *src_file_name, char const *dest_file_name ) {

  /********** open files, read header, ensure source is a Doc file *****/

  FILE *const fin = open_src_file( src_file_name );

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
  int           rec_num;
  DWord         num_offsets, offset;
  unsigned long index;
  int           total_before, total_after;

  File *const fin  = check_fopen( src_file_name, "rb" );
  File *const fout = check_fopen( dest_file_name, "wb" );

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
  header.recordList.numRecords        = htons( num_records + 1 /* rec 0 */ )
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
  for ( rec_num = 1; rec_num <= num_records; ++rec_num ) {
    offset = ftell( fout );
    SEEK_REC_ENTRY( fout, rec_num );
    PUT_DWord( fout, offset );

    int bytes_read;
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
        "  record %2d: %5d bytes -> %5d (%2d%%)\n",
        rec_num, bytes_read, buf.len,
        (int)( 100.0 * buf.len / bytes_read )
      );
      total_before += bytes_read;
      total_after  += buf.len;
    } else
      PRINT_ERR( " %d", num_records - rec_num + 1 );
  } // for
  if ( opt_verbose )
    if ( opt_compress )
      PRINT_ERR( "\
                                  -----\n\
          total compression: %2d%%\n",
        (int)( 100.0 * total_after / total_before )
      );
    else
      putc( '\n', stderr );

  fclose( fin );
  fclose( fout );
}

/**
 * Puts a byte into a buffer.
 *
 * @param b The buffer to be affected.
 * @param c The byte.
 * @param space Is it a space?
 */
void put_byte( buffer_t *b, Byte c, bool *space ) {
  assert( b );
  assert( space );

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

  if ( c >= 1 && c <= 8 || c >= 0x80 )
    b->data[ b->len++ ] = '\1';

  b->data[ b->len++ ] = c;
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
  int i, j;

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

/**
 * Replaces the given buffer with an uncompressed version of itself.
 *
 * @param b The buffer to be uncompressed.
 */
void uncompress( buffer_t *b ) {
  assert( b );

  Byte *const new_data = MALLOC( Byte, BUFFER_SIZE );
  int i, j;

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

/*****************************************************************************
 *
 *  Miscellaneous function(s)
 *
 *****************************************************************************/

static void usage() {
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

/* vim:set et sw=2 ts=2: */
