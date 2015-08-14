/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      options.c
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
#include "util.h"

// standard
#include <ctype.h>
#include <stdlib.h>                     /* for exit() */
#include <string.h>
#include <unistd.h>                     /* for getopt() */

#define GAVE_OPTION(OPT)  isalpha( OPTION_VALUE(OPT) )
#define OPTION_VALUE(OPT) opts_given[ !islower(OPT) ][ toupper(OPT) - 'A' ]
#define SET_OPTION(OPT)   OPTION_VALUE(OPT) = (OPT)

////////// extern variables ///////////////////////////////////////////////////

char const *doc_name;
FILE       *fin;
char const *fin_path;
FILE       *fout;
char const *fout_path = "<stdout>";
char const *me;

bool        opt_binary = true;
bool        opt_compress = true;
bool        opt_decode;
bool        opt_no_check_doc;
bool        opt_no_timestamp;
bool        opt_verbose;

////////// local variables ////////////////////////////////////////////////////

static char opts_given[ 2 /* lower/upper */ ][ 26 + 1 /*NULL*/ ];

////////// local functions ////////////////////////////////////////////////////

/**
 * Checks that if \a opt was given, that at least one \a req_opts was also
 * given.
 * If not, prints an error message and exits.
 *
 * @param opt The short option.
 * @param req_opts The set of required options for \a opt.
 */
static void check_required( char opt, char const *req_opts ) {
  if ( GAVE_OPTION( opt ) ) {
    for ( char const *req_opt = req_opts; *req_opt; ++req_opt )
      if ( GAVE_OPTION( *req_opt ) )
        return;
    bool const reqs_multiple = strlen( req_opts ) > 1;
    PMESSAGE_EXIT( USAGE,
      "-%c requires %sthe -%s option%s to be given also\n",
      opt,
      (reqs_multiple ? "one of " : ""),
      req_opts, (reqs_multiple ? "s" : "")
    );
  }
}

/**
 * Checks that no options were given that are among the two given mutually
 * exclusive sets of short options.
 * Prints an error message and exits if any such options are found.
 *
 * @param opts1 The first set of short options.
 * @param opts2 The second set of short options.
 */
static void check_mutually_exclusive( char const *opts1, char const *opts2 ) {
  int gave_count = 0;
  char const *opt = opts1;
  char gave_opt1 = '\0';

  for ( int i = 0; i < 2; ++i ) {
    for ( ; *opt; ++opt ) {
      if ( GAVE_OPTION( *opt ) ) {
        if ( ++gave_count > 1 ) {
          char const gave_opt2 = *opt;
          PMESSAGE_EXIT( USAGE,
            "-%c and -%c are mutually exclusive\n", gave_opt1, gave_opt2
          );
        }
        gave_opt1 = *opt;
        break;
      }
    } // for
    if ( !gave_count )
      break;
    opt = opts2;
  } // for
}

/**
 * Prints the usage message to standard error and exits.
 */
static void usage( void ) {
  PRINT_ERR(
"usage: %s [-bctv] document_name file.txt file.pdb\n"
"       %s -d [-Dv] file.pdb [file.txt]\n"
"       %s -V\n"
"\n"
"options:\n"
" -b: Don't strip binary characters [default: do].\n"
" -c: Don't compress Doc file [default: do].\n"
" -d: Decode Doc file to text [default: encode to Doc].\n"
" -D: Do not check the type/creator of the Doc file [default: do].\n"
" -t: Do not include timestamps when encoding [default: do].\n"
" -v: Be verbose [default: don't].\n"
" -V: Print version and exit.\n"
    , me, me, me
  );
  exit( EXIT_USAGE );
}

////////// extern functions ///////////////////////////////////////////////////

void process_options( int argc, char *argv[] ) {
  char const opts[] = "bcdDtvV";        // command line options

  me = strrchr( argv[0], '/' );         // determine base name...
  me = me ? me + 1 : argv[0];           // ...of executable

  opterr = 1;
  for ( int opt; (opt = getopt( argc, argv, opts )) != EOF; ) {
    SET_OPTION( opt );
    switch ( opt ) {
      case 'b': opt_binary = false;                 break;
      case 'c': opt_compress = false;               break;
      case 'd': opt_decode = true;                  break;
      case 'D': opt_no_check_doc = true;            break;
      case 't': opt_no_timestamp = true;            break;
      case 'v': opt_verbose = true;                 break;
      case 'V': printf( PACKAGE " " VERSION "\n" ); exit( EXIT_SUCCESS );
      default : usage();
    } // switch
  } // for
  argc -= optind, argv += optind - 1;

  // check for mutually exclusive options
  check_mutually_exclusive( "bct", "dD" );
  check_mutually_exclusive( "V", "bcdDtv" );

  // check for options that require other options
  check_required( 'D', "d" );

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
