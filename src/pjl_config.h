/*
**      PJL Library
**      src/pjl_config.h
**
**      Copyright (C) 2024  Paul J. Lucas
**
**      This program is free software: you can redistribute it and/or modify
**      it under the terms of the GNU General Public License as published by
**      the Free Software Foundation, either version 3 of the License, or
**      (at your option) any later version.
**
**      This program is distributed in the hope that it will be useful,
**      but WITHOUT ANY WARRANTY; without even the implied warranty of
**      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**      GNU General Public License for more details.
**
**      You should have received a copy of the GNU General Public License
**      along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef pjl_config_H
#define pjl_config_H

/**
 * @file
 * Includes platform configuration information in the right order.  Always
 * `#include` this file rather than `config.h` directly.
 */

#ifdef tst2pdbdoc_config_H
#error "Must #include pjl_config.h instead."
#endif /* cdecl_config_H */

// local
#include "config.h"                     /* must go first */

// standard
#include <attribute.h>

////////// compiler attributes ////////////////////////////////////////////////

/**
 * Denote that a function's return value may be discarded without warning.
 *
 * @note There is no compiler attribute for this.  It's just a visual cue in
 * code that `NODISCARD` wasn't forgotten.
 */
#define PJL_DISCARD               /* nothing */

///////////////////////////////////////////////////////////////////////////////

#endif /* pjl_config_H */
/* vim:set et sw=2 ts=2: */
