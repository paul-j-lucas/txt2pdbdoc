/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      doc.h
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

#ifndef txt2pdbdoc_doc_H
#define txt2pdbdoc_doc_H

// local
#include "palm.h"

///////////////////////////////////////////////////////////////////////////////

#define DOC_CREATOR       "REAd"
#define DOC_TYPE          "TEXt"
#define DOC_COMPRESSED    2
#define DOC_UNCOMPRESSED  1

/**
 * Record 0 of a Doc file contains information about the document as a whole.
 */
struct doc_record0 {                    // 16 bytes total
  Word  version;                        // 1 = plain text, 2 = compressed
  Word  reserved1;
  DWord doc_size;                       // in bytes, when uncompressed
  Word  num_records;                    // PDB header numRecords - 1
  Word  rec_size;                       // usually RECORD_SIZE_MAX
  DWord reserved2;
};
typedef struct doc_record0 doc_record0_t;

///////////////////////////////////////////////////////////////////////////////

#endif /* txt2pdbdoc_doc_H */
/* vim:set et sw=2 ts=2: */
