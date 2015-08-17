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

#define FPRINTF(F,...) \
  BLOCK( if ( fprintf( (F), __VA_ARGS__ ) < 0 ) PERROR_EXIT( WRITE_ERROR ); )

#define FSEEK(STREAM,OFFSET,WHENCE) \
  BLOCK( if ( FSEEK_FN( (STREAM), (OFFSET), (WHENCE) ) == -1 ) PERROR_EXIT( SEEK_ERROR ); )

#define FSTAT(FD,STAT) \
  BLOCK( if ( fstat( (FD), (STAT) ) == -1 ) PERROR_EXIT( STAT_ERROR ); )

#define FWRITE(PTR,SIZE,N,STREAM) \
  BLOCK( if ( fwrite( (PTR), (SIZE), (N), (STREAM) ) < (N) ) PERROR_EXIT( WRITE_ERROR ); )

#define MALLOC(TYPE,N)      (TYPE*)check_realloc( NULL, sizeof(TYPE) * (N) )

#define PMESSAGE(FORMAT,...) \
  FPRINTF( stderr, "%s: " FORMAT, me, __VA_ARGS__ )

#define PMESSAGE_EXIT(STATUS,FORMAT,...) \
  BLOCK( PMESSAGE( FORMAT, __VA_ARGS__ ); exit( EXIT_##STATUS ); )

#define UNGETC(C,F) \
  BLOCK( if ( ungetc( (C), (F) ) == EOF ) PERROR_EXIT( READ_ERROR ); )

extern char const* me;                  // executable name from argv[0]

///////////////////////////////////////////////////////////////////////////////

/**
 * Calls \c strdup(3) and checks for failure.
 * If memory allocation fails, prints an error message and exits.
 *
 * @param s The NULL-terminated string to duplicate.
 * @return Returns a copy of \a s.
 */
char* check_strdup( char const *s );

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

/**
 * Adds a pointer to the head of the free-list.
 *  
 * @param p The pointer to add.
 * @return Returns \a p.
 */
void* freelist_add( void *p );

/**
 * Frees all the memory pointed to by all the nodes in the free-list.
 */
void freelist_free( void );

/**
 * TODO
 *
 * @param t TODO
 * @param t_len TODO
 * @param m TODO
 * @param m_len TODO
 * @return TODO
 */
uint8_t* mem_find( uint8_t *t, size_t t_len, uint8_t *m, size_t m_len );

/**
 * Parses a string into a \c uint64_t.
 * Unlike \c strtoull(3), insists that \a s is entirely a non-negative number.
 *
 * @param s The NULL-terminated string to parse.
 * @param n A pointer to receive the parsed number.
 * @return Returns the parsed number only if \a s is entirely a non-negative
 * number or prints an error message and exits if there was an error.
 */
uint64_t parse_ull( char const *s );

/**
 * Peeks at the next character on the given file stream, but does not advance
 * the \c FILE pointer.
 *
 * @param file The file to peek from.
 * @return Returns the next character, if any, or \c EOF if none.
 */
int peekc( FILE *file );

/**
 * Gets a printable version of the given character:
 *  + For characters for which isprint(3) returns non-zero,
 *    the printable version is a single character string of itself.
 *  + For the special-case characters of \0, \a, \b, \f, \n, \r, \t, and \v,
 *    the printable version is a two character string of a backslash followed
 *    by the letter.
 *  + For all other characters, the printable version is a four-character
 *    string of a backslash followed by an 'x' and the two-character
 *    hexedecimal value of che characters ASCII code.
 *
 * @param c The character to get the printable form of.
 * @return Returns a NULL-terminated string that is a printable version of
 * \a c.  Note that the result is a pointer to static storage, hence subsequent
 * calls will overwrite the returned value.  As such, this function is not
 * thread-safe.
 */
char const* printable_char( char c );

///////////////////////////////////////////////////////////////////////////////

#endif /* txt2pdbdoc_util_H */
/* vim:set et sw=2 ts=2: */
