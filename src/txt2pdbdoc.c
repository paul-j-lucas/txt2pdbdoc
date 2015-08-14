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

// standard
#include <stdio.h>
#include <stdlib.h>                     /* for atexit(), exit() */

extern void decode( void );
extern void encode( void );

////////// local functions ////////////////////////////////////////////////////

static void clean_up( void ) {
  if ( fin )
    fclose( fin );
  if ( fout )
    fclose( fout );
}

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

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
