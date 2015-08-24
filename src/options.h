/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      options.h
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

#ifndef txt2pdbdoc_options_H
#define txt2pdbdoc_options_H

// local
#include "util.h"

// standard
#include <stdio.h>                      /* for FILE */

typedef char opts_given_t[ 2 /* lower/upper */ ][ 26 + 1 /*NULL*/ ];

////////// extern variables ///////////////////////////////////////////////////

extern char const  *doc_name;           // document name (when encoding)
extern FILE        *fin;                // file to read from
extern char const  *fin_path;           // path name of input file
extern FILE        *fout;               // file to write to
extern char const  *fout_path;          // path name of output file
extern char const  *me;                 // executable name

extern bool         opt_binary;         // strip binary characters
extern bool         opt_compress;       // compress generated Doc files
extern bool         opt_decode;         // decode from Doc instead
extern bool         opt_no_check_doc;   // don't check Doc file signature
extern bool         opt_no_timestamp;   // don't timestamp generated Doc files
extern bool         opt_no_warnings;    // don't emit character warnings
extern uint32_t     opt_unmapped_codepoint;
extern bool         opt_verbose;        // be verbose
extern opts_given_t opts_given;         // options given

#define GAVE_OPTION(OPT)  isalpha( OPTION_VALUE(OPT) )
#define OPTION_VALUE(OPT) opts_given[ !islower(OPT) ][ toupper(OPT) - 'A' ]
#define SET_OPTION(OPT)   OPTION_VALUE(OPT) = (OPT)

////////// extern functions ///////////////////////////////////////////////////

/**
 * Checks that no options were given that are among the two given mutually
 * exclusive sets of short options.
 * Prints an error message and exits if any such options are found.
 *
 * @param opts1 The first set of short options.
 * @param opts2 The second set of short options.
 */
void check_mutually_exclusive( char const *opts1, char const *opts2 );

/**
 * Parses command-line options and sets global variables.
 *
 * @param argc The argument count from \c main().
 * @param argv The argument values from \c main().
 */
void process_options( int argc, char *argv[] );

///////////////////////////////////////////////////////////////////////////////

#endif /* txt2pdbdoc_options_H */
/* vim:set et sw=2 ts=2: */
