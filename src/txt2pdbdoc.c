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
#include "common.h"
#include "options.h"
#include "util.h"

// standard
#include <stdio.h>
#include <stdlib.h>                     /* for atexit(), exit() */
#include <unistd.h>                     /* for getopt() */

////////// extern declarations ////////////////////////////////////////////////

char const  *doc_name;                  // document name (when encoding)
FILE        *fin;                       // file to read from
char const  *fin_path;                  // path name of input file
FILE        *fout;                      // file to write to
char const  *fout_path;                 // path name of output file
char const  *me;                        // executable name

bool         opt_binary = true;         // strip binary characters
bool         opt_compress = true;       // compress generated Doc files
bool         opt_decode;                // decode from Doc instead
bool         opt_no_check_doc;          // don't check Doc file signature
bool         opt_no_timestamp;          // don't timestamp generated Doc files
bool         opt_no_warnings;           // don't emit character warnings
uint32_t     opt_unmapped_codepoint;    // codepoint to subsitute for unmapped
bool         opt_verbose;               // be verbose

extern void decode( void );
extern void encode( void );

////////// local functions ////////////////////////////////////////////////////

static void clean_up( void );
static void process_options( int, char*[] );
static void usage();

////////// main ///////////////////////////////////////////////////////////////

int main( int argc, char *argv[] ) {
  atexit( clean_up );
  process_options( argc, argv );

  if ( opt_decode )
    decode();
  else
    encode();

  exit( EXIT_SUCCESS );
}

////////// local functions ////////////////////////////////////////////////////

static void clean_up( void ) {
  freelist_free();
  if ( fin )
    fclose( fin );
  if ( fout )
    fclose( fout );
}

/**
 * Prints the usage message to standard error and exits.
 */
static void usage( void ) {
  PRINT_ERR(
"usage: %s [-bctvw] document_name file.txt file.pdb\n"
"       %s -d [-Dvw] [-U codepoint] file.pdb [file.txt]\n"
"       %s -V\n"
"\n"
"options:\n"
"  -b         Don't strip binary characters [default: do].\n"
"  -c         Don't compress generated Doc file [default: do].\n"
"  -d         Decode Doc file to text [default: encode to Doc].\n"
"  -D         Don't check the type/creator of Doc files [default: do].\n"
"  -t         Don't include timestamps when encoding [default: do].\n"
"  -U number  Set Unicode unmapped character [default: none].\n"
"  -v         Be verbose [default: don't].\n"
"  -V         Print version and exit.\n"
"  -w         Don't print character conversion warnings [default: do].\n"
    , me, me, me
  );
  exit( EXIT_USAGE );
}

static void process_options( int argc, char *argv[] ) {
  char const opts[] = "bcdDtU:vVw";     // command line options

  me = strrchr( argv[0], '/' );         // determine base name...
  me = me ? me + 1 : argv[0];           // ...of executable

  opterr = 1;
  for ( int opt; (opt = getopt( argc, argv, opts )) != EOF; ) {
    SET_OPTION( opt );
    switch ( opt ) {
      case 'b': opt_binary = false;                                 break;
      case 'c': opt_compress = false;                               break;
      case 'd': opt_decode = true;                                  break;
      case 'D': opt_no_check_doc = true;                            break;
      case 't': opt_no_timestamp = true;                            break;
      case 'U': opt_unmapped_codepoint = parse_codepoint( optarg ); break;
      case 'v': opt_verbose = true;                                 break;
      case 'V': printf( PACKAGE " " VERSION "\n" );  exit( EXIT_SUCCESS );
      case 'w': opt_no_warnings = true;                             break;
      default : usage();
    } // switch
  } // for
  argc -= optind, argv += optind - 1;

  // check for mutually exclusive options
  check_mutually_exclusive( "bct", "dD" );
  check_mutually_exclusive( "V", "bcdDtUvw" );

  // check for options that require other options
  check_required( 'D', "d" );
  check_required( 'U', "d" );

  if ( opt_decode ) {
    switch ( argc ) {
      case 1:
        fin_path = argv[1];
        fin  = check_fopen( fin_path , "r" );
        fout = stdout;
        break;
      case 2:
        fin_path = argv[1];
        fin = check_fopen( fin_path, "r" );
        fout_path = argv[2];
        fout = check_fopen( fout_path, "w" );
        break;
      default:
        usage();
    } // switch
  } else {
    if ( argc != 3 )
      usage();
    doc_name = argv[1];
    fin_path = argv[2];
    fin = check_fopen( fin_path, "r" );
    fout_path = argv[3];
    fout = check_fopen( fout_path, "w" );
  }
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
