/*
**      Text to Doc converter for Palm Pilots
**      txt2pdbdoc.c
**
**      Copyright (C) 1998  Paul J. Lucas
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
#include "palm.h"

// standard
#include <sys/types.h>                  /* for FreeBSD */
#include <netinet/in.h>                 /* for htonl, etc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* types */
#ifdef  bool
#undef  bool
#endif
#define bool    int

#ifdef  false
#undef  false
#endif
#define false   0

#ifdef  true
#undef  true
#endif
#define true    1

// constants
#define BUFFER_SIZE   6000              /* big enough for uncompressed record */
#define COMPRESSED    2
#define COUNT_BITS    3                 /* why this value?  I don't know */
#define DISP_BITS     11                /* ditto */
#define DOC_CREATOR   "REAd"
#define DOC_TYPE      "TEXt"
#define UNCOMPRESSED  1

// exit status codes
enum {
  Exit_Success      = 0,
  Exit_Usage      = 1,
  Exit_No_Open_Source   = 2,
  Exit_No_Open_Dest   = 3,
  Exit_No_Read      = 4,
  Exit_No_Write     = 5,
  Exit_Not_Doc_File   = 6,
  Exit_Unknown_Compression  = 7
};

// macros
#define NEW_BUFFER(b) (b)->data = malloc( (b)->len = BUFFER_SIZE )

#define GET_Word(f,n) \
  { if ( fread( &n, 2, 1, f ) != 1 ) read_error(); n = ntohs(n); }

#define GET_DWord(f,n) \
  { if ( fread( &n, 4, 1, f ) != 1 ) read_error(); n = ntohl(n); }

#define PUT_Word(f,n) \
  { Word t = htons(n); if ( fwrite( &t, 2, 1, f ) != 1 ) write_error(); }

#define PUT_DWord(f,n) \
  { DWord t = htonl(n); if ( fwrite( &t, 4, 1, f ) != 1 ) write_error(); }

#define SEEK_REC_ENTRY(f,i) \
  fseek( f, DatabaseHdrSize + RecordEntrySize * (i), SEEK_SET )

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
typedef struct doc_record0 doc_record0;

/*****************************************************************************
 *
 *  Globals
 *
 *****************************************************************************/

typedef struct {
  Byte    *data;
  unsigned len;
} buffer;

char const* me;                         // executable name

bool    binary_opt    = true;
bool    compress_opt    = true;
bool    no_check_doc_opt  = false;
bool    verbose_opt   = false;

void    compress( buffer* );
void    decode( char const*, char const* );
void    encode( char const*, char const*, char const* );
Byte*   mem_find( Byte*, int, Byte*, int );
FILE*   open_src_file ( char const* );
FILE*   open_dest_file( char const* );
void    put_byte( buffer*, Byte, bool* );
void    read_error();
void    remove_binary( buffer* );
void    uncompress( buffer* );
void    usage();
void    write_error();

/*****************************************************************************
 *
 * SYNOPSIS
 */
  int main( int argc, char *argv[] )
/*
 * DESCRIPTION
 *
 *  Parse the command line, initialize, call other functions ... the
 *  usual things that are done in main().
 *
 * PARAMETERS
 *
 *  argc  The number of arguments.
 *
 *  argv  A vector of the arguments; argv[argc] is null.  Aside from
 *    the options below, the arguments are the names of the files.
 *
 * SEE ALSO
 *
 *  Brian W. Kernighan, Dennis M. Ritchie.  "The C Programming Language,
 *  2nd ed."  Addison-Wesley, Reading, MA.  pp. 114-118.
 *
 *****************************************************************************/
{
  extern char*  optarg;
  extern int  optind, opterr;

  bool        decode_opt = false;
  char const  opts[] = "bcdDvV";        // command line options
  int         opt;                      // option being processed

  me = strrchr( argv[0], '/' );   /* determine base name... */
  me = me ? me + 1 : argv[0];   /* ...of executable */

  /********** Process command-line options *****************************/

  opterr = 1;
  while ( (opt = getopt( argc, argv, opts )) != EOF ) {
    switch ( opt ) {
      case 'b': binary_opt = false;                 break;
      case 'c': compress_opt = false;               break;
      case 'd': decode_opt = true;                  break;
      case 'D': no_check_doc_opt = true;            break;
      case 'v': verbose_opt = true;                 break;
      case 'V': printf( PACKAGE " " VERSION "\n" ); exit( Exit_Success );
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

  exit( Exit_Success );
}

/*****************************************************************************
 *
 * SYNOPSIS
 */
  void compress( buffer *b )
/*
 * DESCRIPTION
 *
 *  Replace the given buffer with a compressed version of itself.  I don't
 *  understand this algorithm.  I just cleaned up the code.
 *
 * PARAMETERS
 *
 *  b The buffer to be compressed.
 *
 *****************************************************************************/
{
  int i, j;
  bool space = false;

  Byte *buf_orig;
  Byte *p;                  // walking test hit; works up on successive matches
  Byte *p_prev;
  Byte *head;                           // current test string
  Byte *tail;                           // 1 past the current test buffer
  Byte *end;                            // 1 past the end of the input buffer

  p = p_prev = head = buf_orig = b->data;
  tail = head + 1;
  end = b->data + b->len;

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
    if ( !p || p == head || tail - head > ( 1 << COUNT_BITS ) + 2 ||
         tail == end ) {
      // issued the codes
      // first, check for short runs
      if ( tail - head < 4 )
        put_byte( b, *head++, &space );
      else {
        unsigned dist = head - p_prev;
        unsigned compound = (dist << COUNT_BITS) + tail - head - 4;

        if ( dist >= ( 1 << DISP_BITS ) || tail - head - 4 > 7 )
          fprintf( stderr, "%s: error: dist overflow\n", me );

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

/*****************************************************************************
 *
 * SYNOPSIS
 */
  void decode( char const *src_file_name, char const *dest_file_name )
/*
 * DESCRIPTION
 *
 *  Decode the source Doc file to a text file.
 *
 * PARAMETERS
 *
 *  src_file_name The name of the Doc file.
 *
 *  dest_file_name  The name of the text file.  If null, text is sent to
 *      standard output.
 *
 *****************************************************************************/
{
  buffer    buf;
  int   compression;
  DWord   file_size, offset, rec_size;
  FILE    *fin, *fout;
  DatabaseHdrType header;
  int   num_records, rec_num;
  doc_record0 rec0;

  /********** open files, read header, ensure source is a Doc file *****/

  fin = open_src_file( src_file_name );

  if ( fread( &header, DatabaseHdrSize, 1, fin ) != 1 )
    read_error();
  if ( !no_check_doc_opt && (
       strncmp( header.type,    DOC_TYPE,    sizeof header.type ) ||
       strncmp( header.creator, DOC_CREATOR, sizeof header.creator )
  ) ) {
    fprintf( stderr, "%s: %s is not a Doc file\n", me, src_file_name );
    exit( Exit_Not_Doc_File );
  }

  num_records = ntohs( header.recordList.numRecords ) - 1; /* w/o rec 0 */

  fout = dest_file_name ? open_dest_file( dest_file_name ) : stdout;

  /********** read record 0 ********************************************/

  SEEK_REC_ENTRY( fin, 0 );
  GET_DWord( fin, offset );   /* get offset of rec 0 */
  fseek( fin, offset, SEEK_SET );
  if ( fread( &rec0, sizeof rec0, 1, fin ) != 1 )
    read_error();

  compression = ntohs( rec0.version );
  if ( compression != COMPRESSED && compression != UNCOMPRESSED ) {
    fprintf( stderr,
      "%s: error: unknown file compression type: %d\n",
      me, compression
    );
    exit( Exit_Unknown_Compression );
  }

  /********* read Doc file record-by-record ****************************/

  fseek( fin, 0, SEEK_END );
  file_size = ftell( fin );

  if ( verbose_opt )
    fprintf( stderr, "%s: decoding \"%s\":", me, header.name );

  NEW_BUFFER( &buf );
  for ( rec_num = 1; rec_num <= num_records; ++rec_num ) {
    DWord next_offset;

    /* read the record offset */
    SEEK_REC_ENTRY( fin, rec_num );
    GET_DWord( fin, offset );

    /* read the next record offset to compute the record size */
    if ( rec_num < num_records ) {
      SEEK_REC_ENTRY( fin, rec_num + 1 );
      GET_DWord( fin, next_offset );
    } else
      next_offset = file_size;
    rec_size = next_offset - offset;

    /* read the record */
    fseek( fin, offset, SEEK_SET );
    buf.len = fread( buf.data, 1, rec_size, fin );
    if ( buf.len != rec_size )
      read_error();

    if ( compression == COMPRESSED )
      uncompress( &buf );

    if ( fwrite( buf.data, buf.len, 1, fout ) != 1 )
      write_error();
    if ( verbose_opt )
      fprintf( stderr, " %d", num_records - rec_num );
  } // for
  if ( verbose_opt )
    putc( '\n', stderr );

  fclose( fin );
  fclose( fout );
}

/*****************************************************************************
 *
 * SYNOPSIS
 */
  void encode( char const *document_name,
    char const *src_file_name, char const *dest_file_name
  )
/*
 * DESCRIPTION
 *
 *  Encode the source text file into a Doc file.
 *
 * PARAMETERS
 *
 *  document_name The name of the document as it is to appear in the
 *      Documents List view of a Doc reader application on
 *      the Pilot.
 *
 *  src_file_name The name of the text file.
 *
 *  dest_file_name  The name of the Doc file.
 *
 *****************************************************************************/
{
  DWord   date;
  FILE    *fin, *fout;
  DWord   file_size;
  DatabaseHdrType header;
  doc_record0 rec0;
  buffer    buf;
  int   num_records, rec_num;
  DWord   num_offsets, offset;
  unsigned long index;
  int   total_before, total_after;

  fin  = open_src_file ( src_file_name  );
  fout = open_dest_file( dest_file_name );

  fseek( fin, 0, SEEK_END );
  file_size = ftell( fin );
  num_records = file_size / RECORD_SIZE_MAX;
  if ( (long)num_records * RECORD_SIZE_MAX < file_size )
    ++num_records;

  /********** create and write header **********************************/

  bzero( header.name, sizeof header.name );
  strncpy( header.name, document_name, sizeof header.name - 1 );
  if ( strlen( document_name ) > sizeof header.name - 1 )
    strncpy( header.name + sizeof header.name - 4, "...", 3 );
  header.attributes     = 0;
  header.version        = 0;
  date = htonl( palm_date() );
  memcpy( &header.creationDate,   &date, 4 );
  date = htonl( palm_date() );
  memcpy( &header.modificationDate, &date, 4 );
  header.lastBackupDate     = 0;
  header.modificationNumber   = 0;
  header.appInfoID      = 0;
  header.sortInfoID     = 0;
  strncpy( header.type,    DOC_TYPE,    sizeof header.type );
  strncpy( header.creator, DOC_CREATOR, sizeof header.creator );
  header.uniqueIDSeed     = 0;
  header.recordList.nextRecordListID  = 0;
  header.recordList.numRecords    = htons( num_records + 1 );
            /* +1 for rec 0 */
  fseek( fin, 0, SEEK_SET );
  if ( fwrite( &header, DatabaseHdrSize, 1, fout ) != 1 )
    write_error();

  /********** write record offsets *************************************/

  num_offsets = num_records + 1;    /* +1 for rec 0 */
  offset = DatabaseHdrSize + RecordEntrySize * num_offsets;
  index = 0x40 << 24 | 0x6F8000;    /* dirty + unique ID */

  PUT_DWord( fout, offset );    /* offset for rec 0 */
  PUT_DWord( fout, index++ );

  while( --num_offsets ) {
    PUT_DWord( fout, 0 );   /* placeholder */
    PUT_DWord( fout, index++ );
  }

  /********** write record 0 *******************************************/

  rec0.version    = htons( compress_opt + 1 );
  rec0.reserved1    = 0;
  rec0.doc_size   = htonl( file_size );
  rec0.num_records  = htons( num_records );
  rec0.rec_size   = htons( RECORD_SIZE_MAX );
  rec0.reserved2    = 0;

  if ( fwrite( &rec0, sizeof rec0, 1, fout ) != 1 )
    write_error();

  /********** write text ***********************************************/

  NEW_BUFFER( &buf );
  total_before = total_after = 0;
  for ( rec_num = 1; rec_num <= num_records; ++rec_num ) {
    int bytes_read;

    offset = ftell( fout );
    SEEK_REC_ENTRY( fout, rec_num );
    PUT_DWord( fout, offset );

    if ( !(bytes_read = fread( buf.data, 1, RECORD_SIZE_MAX, fin )))
      break;
    if ( ferror( fin ) )
      read_error();
    buf.len = bytes_read;

    if ( binary_opt )
      remove_binary( &buf );
    if ( compress_opt )
      compress( &buf );

    fseek( fout, offset, SEEK_SET );
    if ( fwrite( buf.data, buf.len, 1, fout ) != 1 )
      write_error();

    if ( !verbose_opt )
      continue;

    if ( compress_opt ) {
      fprintf( stderr,
        "  record %2d: %5d bytes -> %5d (%2d%%)\n",
        rec_num, bytes_read, buf.len,
        (int)( 100.0 * buf.len / bytes_read )
      );
      total_before += bytes_read;
      total_after  += buf.len;
    } else
      fprintf( stderr, " %d", num_records - rec_num + 1 );
  } // for
  if ( verbose_opt )
    if ( compress_opt )
      fprintf( stderr, "\
                                  -----\n\
          total compression: %2d%%\n",
        (int)( 100.0 * total_after / total_before )
      );
    else
      putc( '\n', stderr );

  fclose( fin );
  fclose( fout );
}

/*****************************************************************************
 *
 * SYNOPSIS
 */
  void put_byte( register buffer *b, Byte c, bool *space )
/*
 * DESCRIPTION
 *
 *  Put a byte into a buffer.
 *
 * PARAMETERS
 *
 *  b The buffer to be affected.
 *
 *  c The byte.
 *
 *  space Is it a space?
 *
 *****************************************************************************/
{
  if ( *space ) {
    *space = false;
    /*
    ** There is an outstanding space char: see if we can squeeze it
    ** in with an ASCII char.
    */
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

/*****************************************************************************
 *
 * SYNOPSIS
 */
  void remove_binary( register buffer *b )
/*
 * DESCRIPTION
 *
 *  Replace the given buffer with one that has had ASCII characters (0-8)
 *  removed and carriage-returns and form-feeds converted to newlines.
 *
 * PARAMETERS
 *
 *  b The buffer to be affected.
 *
 *****************************************************************************/
{
  Byte *const new_data = malloc( b->len );
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

/*****************************************************************************
 *
 * SYNOPSIS
 */
  void uncompress( register buffer *b )
/*
 * DESCRIPTION
 *
 *  Replace the given buffer with an uncompressed version of itself.
 *
 * PARAMETERS
 *
 *  b The buffer to be uncompressed.
 *
 *****************************************************************************/
{
  Byte *const new_data = malloc( BUFFER_SIZE );
  int i, j;

  for ( i = j = 0; i < b->len; ) {
    register unsigned c = b->data[ i++ ];

    if ( c >= 1 && c <= 8 )
      while ( c-- )                     // copy 'c' bytes
        new_data[ j++ ] = b->data[ i++ ];

    else if ( c <= 0x7F )               // 0,09-7F = self
      new_data[ j++ ] = c;

    else if ( c >= 0xC0 )               // space + ASCII char
      new_data[ j++ ] = ' ', new_data[ j++ ] = c ^ 0x80;

    else {                              // 80-BF = sequences
      register int di, n;
      c = (c << 8) + b->data[ i++ ];
      di = (c & 0x3FFF) >> COUNT_BITS;
      for ( n = (c & ((1 << COUNT_BITS) - 1)) + 3; n--; ++j )
        new_data[ j ] = new_data[ j - di ];
    }
  }
  free( b->data );
  b->data = new_data;
  b->len = j;
}

/*****************************************************************************
 *
 *  Miscellaneous function(s)
 *
 *****************************************************************************/

/* replacement for strstr() that deals with 0's in the data */
Byte* mem_find( register Byte *t, int t_len, register Byte *m, int m_len ) {
  register int i;
  for ( i = t_len - m_len + 1; i > 0; --i, ++t )
    if ( *t == *m && !memcmp( t, m, m_len ) )
      return t;
  return 0;
}

FILE* open_src_file( char const *file_name ) {
  FILE *f = fopen( file_name, "rb" );
  if ( f ) return f;
  fprintf( stderr, "%s: can not open %s for input\n", me, file_name );
  exit( Exit_No_Open_Source );
}

FILE* open_dest_file( char const *file_name ) {
  FILE *f = fopen( file_name, "wb" );
  if ( f ) return f;
  fprintf( stderr, "%s: can not open %s for output\n", me, file_name );
  exit( Exit_No_Open_Dest );
}

void read_error() {
  fprintf( stderr, "%s: reading failed\n", me );
  exit( Exit_No_Read );
}

void usage() {
  fprintf( stderr,
  "usage: %s [-c] [-b] [-v] document_name file.txt file.pdb\n"
  "       %s  -d  [-D] [-v] file.pdb [file.txt]\n"
  "       %s  -V\n\n"
  "options:\n"
  "--------\n"
  " -b: don't strip binary characters\n"
  " -c: don't compress Doc file\n"
  " -d: decode Doc file to text\n"
  " -D: do not check the type/creator of the Doc file\n"
  " -v: verbose\n"
  " -V: print version and exit\n", me, me, me );
  exit( Exit_Usage );
}

void write_error() {
  fprintf( stderr, "%s: writing failed\n", me );
  exit( Exit_No_Write );
}
/* vim:set et sw=2 ts=2: */
