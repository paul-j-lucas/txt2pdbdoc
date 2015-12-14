/*
**      pdbdump -- Palm database dumper for Palm Pilots
**      pdbdump.c
**
**      Copyright (C) 2001-2015  Paul J. Lucas
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
#include <ctype.h>
#include <sys/types.h>
#include <arpa/inet.h>                  /* for ntohs(), ntohl() */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>                     /* for getopt() */

#define BUF_SIZE  4096
#define ROW_SIZE  16                    /* bytes per dump row */

////////// extern variables ///////////////////////////////////////////////////

char const  *me;

////////// local variables ////////////////////////////////////////////////////

static FILE  *fin;                      // file to read from
static FILE  *fout;                     // file to write to

static bool   opt_header_only = false;
static bool   opt_data_only = false;

////////// local functions ////////////////////////////////////////////////////

static void clean_up( void );
static void dump_pdb_header( DatabaseHdrType const* );
static void dump_rec_header( Word, RecordEntryType const* );
static void dump_row( DWord, uint8_t const*, DWord );
static void process_options( int, char*[] );
static void usage( void );

////////// main ///////////////////////////////////////////////////////////////

int main( int argc, char *argv[] ) {
  atexit( clean_up );
  process_options( argc, argv );

  ////////// read PDB header //////////////////////////////////////////////////

  DatabaseHdrType hdr;
  FREAD( &hdr, DatabaseHdrSize, 1, fin );

  if ( !opt_data_only ) {
    dump_pdb_header( &hdr );
    if ( opt_header_only )
      exit( EXIT_SUCCESS );
  }

  ////////// read records /////////////////////////////////////////////////////

  Word const num_records = ntohs( hdr.recordList.numRecords );
  for ( Word rec_num = 0; rec_num < num_records; ++rec_num ) {

    // read record
    RecordEntryType rec;
    SEEK_REC( fin, rec_num );
    FREAD( &rec, RecordEntrySize, 1, fin );

    if ( !opt_data_only ) {
      dump_rec_header( rec_num, &rec );
      if ( opt_header_only )
        exit( EXIT_SUCCESS );
    }

    // get offset to record payload
    DWord offset = ntohl( rec.offset ), next_offset;
    if ( rec_num < num_records - 1 ) {
      SEEK_REC( fin, rec_num + 1 );
      FREAD( &rec, RecordEntrySize, 1, fin );
      next_offset = ntohl( rec.offset );
    } else {
      FSEEK( fin, 0, SEEK_END );
      next_offset = ftell( fin );
    }

    // read record payload
    FSEEK( fin, offset, SEEK_SET );
    uint8_t buf[ BUF_SIZE ];
    FREAD( buf, 1, next_offset - offset + 1, fin );
    uint8_t const *pbuf = buf;

    while ( offset < next_offset ) {
      DWord row_len = next_offset - offset;
      if ( row_len > ROW_SIZE )
        row_len = ROW_SIZE;
      dump_row( offset, pbuf, row_len );
      offset += ROW_SIZE;
      pbuf += ROW_SIZE;
    } // while
  } // for

  exit( EXIT_SUCCESS );
}

////////// miscellaneous functions ////////////////////////////////////////////

static void clean_up( void ) {
  freelist_free();
  if ( fin )
    fclose( fin );
  if ( fout )
    fclose( fout );
}

static void dump_pdb_header( DatabaseHdrType const *hdr ) {
  FPRINTF( fout,
    "   Name: %s\n"
    "Version: %d\n"
    "   Type: %c%c%c%c\n"
    "Creator: %c%c%c%c\n"
    "Records: %d\n",

    hdr->name,
    ntohs( hdr->version ),
    hdr->type[0], hdr->type[1], hdr->type[2], hdr->type[3],
    hdr->creator[0], hdr->creator[1], hdr->creator[2], hdr->creator[3],
    ntohs( hdr->recordList.numRecords )
  );
}

static void dump_rec_header( Word rec_num, RecordEntryType const *rec ) {
  FPRINTF( fout,
    "===================================================================\n"
    "Rec %4d: [%c] Delete [%c] Dirty [%c] Busy [%c] Secret\n"
    "-------------------------------------------------------------------\n",
    rec_num,
    rec->attributes.delete ? 'X' : ' ',
    rec->attributes.dirty  ? 'X' : ' ',
    rec->attributes.busy   ? 'X' : ' ',
    rec->attributes.secret ? 'X' : ' '
  );
}

static void dump_row( DWord offset, uint8_t const *row, DWord row_len ) {
  // print offset
  FPRINTF( fout, "%08X:", offset );

  // print hex part
  DWord row_pos;
  for ( row_pos = 0; row_pos < row_len; ++row_pos ) {
    if ( row_pos % 2 == 0 )
      FPUTC( ' ', fout );
    FPRINTF( fout, "%02X", (unsigned)row[ row_pos ] );
  } // for

  // print padding if necessary (last row only)
  while ( row_pos < ROW_SIZE ) {
    if ( row_pos++ % 2 == 0 )
      FPUTC( ' ', fout );
    FPRINTF( fout, "  " );
  } // while

  // print ASCII part
  FPRINTF( fout, "  " );
  for ( row_pos = 0; row_pos < row_len; ++row_pos )
    FPUTC( isprint( row[ row_pos ] ) ? row[ row_pos ] : '.', fout );
  FPUTC( '\n', fout );
}

static void process_options( int argc, char *argv[] ) {
  char const opts[] = "dhV";            // command line options

  me = strrchr( argv[0], '/' );         // determine base name...
  me = me ? me + 1 : argv[0];           // ...of executable

  opterr = 1;
  for ( int opt; (opt = getopt( argc, argv, opts )) != EOF; ) {
    SET_OPTION( opt );
    switch ( opt ) {
      case 'd': opt_data_only = true;               break;
      case 'h': opt_header_only = true;             break;
      case 'V': printf( "pdbdump %s\n", VERSION );  exit( EXIT_SUCCESS );
      default : usage();
    } // switch
  } // for
  argc -= optind, argv += optind - 1;

  check_mutually_exclusive( "d", "h" );
  check_mutually_exclusive( "V", "dh" );

  switch ( argc ) {
    case 1:
      fin  = check_fopen( argv[1], "rb" );
      fout = stdout;
      break;
    case 2:
      fin  = check_fopen( argv[1], "rb" );
      fout = check_fopen( argv[2], "wb" );
      break;
    default:
      usage();
  } // switch
}

/**
 * Prints the usage message to standard error and exits.
 */
static void usage() {
  PRINT_ERR(
"usage: %s [-dhV] file.pdb [file.txt]\n"
"\n"
"options:\n"
"  -d  Dump data only.\n"
"  -h  Dump header only.\n"
"  -V  Print version and exit.\n"
    , me
  );
  exit( EXIT_USAGE );
}

///////////////////////////////////////////////////////////////////////////////
// vim:set et sw=2 ts=2:
