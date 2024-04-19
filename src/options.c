/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      options.c
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
#include "pjl_config.h"
#include "common.h"
#include "options.h"
#include "util.h"

// standard
#include <assert.h>
#include <stdbool.h>
#include <sysexits.h>

////////// extern declarations ////////////////////////////////////////////////

bool opts_given[ 128 ];

////////// extern functions ///////////////////////////////////////////////////

void check_required( char const *opts, char const *req_opts ) {
  assert( opts != NULL );
  assert( req_opts != NULL );

  for ( char const *opt = opts; *opt; ++opt ) {
    if ( opts_given[ STATIC_CAST( unsigned, *opt ) ] ) {
      for ( char const *req_opt = req_opts; *req_opt; ++req_opt ) {
        if ( opts_given[ STATIC_CAST( unsigned, *req_opt ) ] )
          return;
      } // for
      bool const reqs_multiple = strlen( req_opts ) > 1;
      PMESSAGE_EXIT( EX_USAGE,
        "-%c requires %sthe -%s option%s to be given also\n",
        *opt, (reqs_multiple ? "one of " : ""),
        req_opts, (reqs_multiple ? "s" : "")
      );
    }
  } // for
}

void check_mutually_exclusive( char const *opts1, char const *opts2 ) {
  assert( opts1 != NULL );
  assert( opts2 != NULL );

  unsigned gave_count = 0;
  char const *opt = opts1;
  char gave_opt1 = '\0';

  for ( unsigned i = 0; i < 2; ++i ) {
    for ( ; *opt; ++opt ) {
      if ( opts_given[ STATIC_CAST( unsigned, *opt ) ] ) {
        if ( ++gave_count > 1 ) {
          char const gave_opt2 = *opt;
          PMESSAGE_EXIT( EX_USAGE,
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

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
