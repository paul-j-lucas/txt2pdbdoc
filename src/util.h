/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      util.h
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

#ifndef txt2pdbdoc_util_H
#define txt2pdbdoc_util_H

// local
#include "config.h"

// standard
#include <sys/types.h>                  /* for FreeBSD */
#include <errno.h>
#include <stddef.h>                     /* for size_t */
#include <stdint.h>                     /* for uint8_t */
#include <stdio.h>                      /* for FILE */
#include <stdlib.h>
#include <string.h>                     /* for strerror() */

///////////////////////////////////////////////////////////////////////////////

// define a "bool" type
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# ifndef HAVE__BOOL
#   ifdef __cplusplus
typedef bool _Bool;
#   else
#     define _Bool signed char
#   endif /* __cplusplus */
# endif /* HAVE__BOOL */
# define bool   _Bool
# define false  0
# define true   1
# define __bool_true_false_are_defined 1
#endif /* HAVE_STDBOOL_H */

#ifdef HAVE_FSEEKO
# define FSEEK_FN fseeko
#else
# define FSEEK_FN fseek
#endif /* HAVE_FSEEKO */

#define BLOCK(...)          do { __VA_ARGS__ } while (0)
#define PERROR_EXIT(STATUS) BLOCK( perror( me ); exit( EXIT_##STATUS ); )
#define PRINT_ERR(...)      fprintf( stderr, __VA_ARGS__ )
#define STRERROR            strerror( errno )

#define FREAD(PTR,SIZE,N,STREAM) \
  BLOCK( if ( fread( (PTR), (SIZE), (N), (STREAM) ) < (N) ) PERROR_EXIT( READ_ERROR ); )

#define FSEEK(STREAM,OFFSET,WHENCE) \
  BLOCK( if ( FSEEK_FN( (STREAM), (OFFSET), (WHENCE) ) == -1 ) PERROR_EXIT( SEEK_ERROR ); )

#define FWRITE(PTR,SIZE,N,STREAM) \
  BLOCK( if ( fwrite( (PTR), (SIZE), (N), (STREAM) ) < (N) ) PERROR_EXIT( WRITE_ERROR ); )

#define MALLOC(TYPE,N)      (TYPE*)check_realloc( NULL, sizeof(TYPE) * (N) )

#define PMESSAGE_EXIT(STATUS,FORMAT,...) \
  BLOCK( PRINT_ERR( "%s: " FORMAT, me, __VA_ARGS__ ); exit( EXIT_##STATUS ); )

extern char const* me;                  // executable name from argv[0]

///////////////////////////////////////////////////////////////////////////////

/**
 * Opens the given file and seeks to the given offset
 * or prints an error message and exits if there was an error.
 *
 * @param path The full path of the file to open.
 * @param mode The mode to use.
 * @return Returns the corresponding \c FILE.
 */
FILE* check_fopen( char const *path, char const *mode );

/**
 * Calls \c realloc(3) and checks for failure.
 * If reallocation fails, prints an error message and exits.
 *
 * @param p The pointer to reallocate.  If NULL, new memory is allocated.
 * @param size The number of bytes to allocate.
 * @return Returns a pointer to the allocated memory.
 */
void* check_realloc( void *p, size_t size );

uint8_t* mem_find( uint8_t *t, size_t t_len, uint8_t *m, size_t m_len );

///////////////////////////////////////////////////////////////////////////////

#endif /* txt2pdbdoc_util_H */
/* vim:set et sw=2 ts=2: */
