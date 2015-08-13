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
#include <stdlib.h>                     /* for exit() */
#include <string.h>
#include <unistd.h>                     /* for getopt() */

///////////////////////////////////////////////////////////////////////////////

char const *me;                         // executable name
bool        opt_binary = true;
bool        opt_compress = true;
bool        opt_decode = false;
bool        opt_no_check_doc = false;
bool        opt_verbose = false;

////////// local functions ////////////////////////////////////////////////////

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

////////// extern functions ///////////////////////////////////////////////////

void process_options( int argc, char *argv[] ) {
  char const opts[] = "bcdDvV";         // command line options

  me = strrchr( argv[0], '/' );         // determine base name...
  me = me ? me + 1 : argv[0];           // ...of executable

  opterr = 1;
  for ( int opt; (opt = getopt( argc, argv, opts )) != EOF; ) {
    switch ( opt ) {
      case 'b': opt_binary = false;                 break;
      case 'c': opt_compress = false;               break;
      case 'd': opt_decode = true;                  break;
      case 'D': opt_no_check_doc = true;            break;
      case 'v': opt_verbose = true;                 break;
      case 'V': printf( PACKAGE " " VERSION "\n" ); exit( EXIT_SUCCESS );
      default : usage();
    } // switch
  } // for
  argc -= optind, argv += optind;

  if ( opt_decode ) {
    if ( argc < 1 || argc > 2 )
      usage();
  } else {
    if ( argc != 3 )
      usage();
  }
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
