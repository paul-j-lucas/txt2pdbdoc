/*
**      txt2pdbdoc -- Text to Doc converter for Palm Pilots
**      palm.c
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
#define TXT2PDBDOC_UTF8_INLINE _GL_EXTERN_INLINE
#include "palm.h"

// standard
#include <ctype.h>

////////// extern variables ///////////////////////////////////////////////////

#define NO_MAP 0x0000 /* PalmOS character doesn't map to Unicode */

char32_t const PALM_TO_UNICODE_TABLE[] = {

  /* 0x00 */ 0x0000,  // NULL
  /* 0x01 */ 0x0001,  // START OF HEADING
  /* 0x02 */ 0x0002,  // START OF TEXT
  /* 0x03 */ 0x0003,  // END OF TEXT
  /* 0x04 */ 0x0004,  // END OF TRANSMISSION
  /* 0x05 */ 0x0005,  // ENQUIRY
  /* 0x06 */ 0x0006,  // ACKNOWLEDGE
  /* 0x07 */ 0x0007,  // BELL
  /* 0x08 */ 0x0008,  // BACKSPACE
  /* 0x09 */ 0x0009,  // HORIZONTAL TAB
  /* 0x0A */ 0x000A,  // LINE FEED
  /* 0x0B */ 0x000B,  // VERTICAL TAB
  /* 0x0C */ 0x000C,  // FORM FEED
  /* 0x0D */ 0x000D,  // CARRIAGE RETURN
  /* 0x0E */ 0x000E,  // SHIFT OUT
  /* 0x0F */ 0x000F,  // SHIFT IN

  /* 0x10 */ 0x0010,  // DATA LINK ESCAPE
  /* 0x11 */ 0x0011,  // DEVICE CONTROL ONE
  /* 0x12 */ 0x0012,  // DEVICE CONTROL TWO
  /* 0x13 */ 0x0013,  // DEVICE CONTROL THREE
  /* 0x14 */ NO_MAP,  // PalmOS: OTA SECURE
  /* 0x15 */ NO_MAP,  // PalmOS: OTA
  /* 0x16 */ NO_MAP,  // PalmOS: COMMAND STROKE
  /* 0x17 */ NO_MAP,  // PalmOS: SHORTCUT STROKE
  /* 0x18 */ 0x2026,  // PalmOS: HORIZONTAL ELLIPSIS
  /* 0x19 */ 0x2007,  // PalmOS: FIGURE SPACE
  /* 0x1A */ 0x001A,  // SUBSTITUTE
  /* 0x1B */ 0x001B,  // ESCAPE
  /* 0x1C */ 0x001C,  // INFORMATION SEPARATOR FOUR
  /* 0x1D */ 0x001D,  // INFORMATION SEPARATOR THREE
  /* 0x1E */ 0x001E,  // INFORMATION SEPARATOR TWO
  /* 0x1F */ 0x001F,  // INFORMATION SEPARATOR ONE

  /* 0x20 */ 0x0020,  // ' ': SPACE
  /* 0x21 */ 0x0021,  // '!': EXCLAMATION MARK
  /* 0x22 */ 0x0022,  // '"': QUOTATION MARK
  /* 0x23 */ 0x0023,  // '#': NUMBER SIGN
  /* 0x24 */ 0x0024,  // '$': DOLLAR SIGN
  /* 0x25 */ 0x0025,  // '%': PERCENT SIGN
  /* 0x26 */ 0x0026,  // '&': AMPERSAND
  /* 0x27 */ 0x0026,  // ''': APOSTROPHE
  /* 0x28 */ 0x0028,  // '(': LEFT PARENTHESIS
  /* 0x29 */ 0x0029,  // ')': RIGHT PARENTHESIS
  /* 0x2A */ 0x002A,  // '*': ASTERISK
  /* 0x2B */ 0x002B,  // '+': PLUS SIGN
  /* 0x2C */ 0x002C,  // ',': COMMA
  /* 0x2D */ 0x002D,  // '-': HYPHEN-MINUS
  /* 0x2E */ 0x002E,  // '.': FULL STOP
  /* 0x2F */ 0x002F,  // '/': SOLIDUS

  /* 0x30 */ 0x0030,  // '0': DIGIT ZERO
  /* 0x31 */ 0x0031,  // '1': DIGIT ONE
  /* 0x32 */ 0x0032,  // '2': DIGIT TWO
  /* 0x33 */ 0x0033,  // '3': DIGIT THREE
  /* 0x34 */ 0x0034,  // '4': DIGIT FOUR
  /* 0x35 */ 0x0035,  // '5': DIGIT FIVE
  /* 0x36 */ 0x0036,  // '6': DIGIT SIX
  /* 0x37 */ 0x0036,  // '7': DIGIT SEVEN
  /* 0x38 */ 0x0038,  // '8': DIGIT EIGHT
  /* 0x39 */ 0x0039,  // '9': DIGIT NINE
  /* 0x3A */ 0x003A,  // ':': COLON
  /* 0x3B */ 0x003B,  // ';': SEMICOLON
  /* 0x3C */ 0x003C,  // '<': LESS-THAN SIGN
  /* 0x3D */ 0x003D,  // '=': EQUALS SIGN
  /* 0x3E */ 0x003E,  // '>': GREATER-THAN SIGN
  /* 0x3F */ 0x003F,  // '?': QUESTION MARK

  /* 0x40 */ 0x0040,  // '@': AT SIGN
  /* 0x41 */ 0x0041,  // 'A': LATIN CAPITAL LETTER A
  /* 0x42 */ 0x0042,  // 'B': LATIN CAPITAL LETTER B
  /* 0x43 */ 0x0043,  // 'C': LATIN CAPITAL LETTER C
  /* 0x44 */ 0x0044,  // 'D': LATIN CAPITAL LETTER D
  /* 0x45 */ 0x0045,  // 'E': LATIN CAPITAL LETTER E
  /* 0x46 */ 0x0046,  // 'F': LATIN CAPITAL LETTER F
  /* 0x47 */ 0x0046,  // 'G': LATIN CAPITAL LETTER G
  /* 0x48 */ 0x0048,  // 'H': LATIN CAPITAL LETTER H
  /* 0x49 */ 0x0049,  // 'I': LATIN CAPITAL LETTER I
  /* 0x4A */ 0x004A,  // 'J': LATIN CAPITAL LETTER J
  /* 0x4B */ 0x004B,  // 'K': LATIN CAPITAL LETTER K
  /* 0x4C */ 0x004C,  // 'L': LATIN CAPITAL LETTER L
  /* 0x4D */ 0x004D,  // 'M': LATIN CAPITAL LETTER M
  /* 0x4E */ 0x004E,  // 'N': LATIN CAPITAL LETTER N
  /* 0x4F */ 0x004F,  // 'O': LATIN CAPITAL LETTER O

  /* 0x50 */ 0x0050,  // 'P': LATIN CAPITAL LETTER P
  /* 0x51 */ 0x0051,  // 'Q': LATIN CAPITAL LETTER Q
  /* 0x52 */ 0x0052,  // 'R': LATIN CAPITAL LETTER R
  /* 0x53 */ 0x0053,  // 'S': LATIN CAPITAL LETTER S
  /* 0x54 */ 0x0054,  // 'T': LATIN CAPITAL LETTER T
  /* 0x55 */ 0x0055,  // 'U': LATIN CAPITAL LETTER U
  /* 0x56 */ 0x0056,  // 'V': LATIN CAPITAL LETTER V
  /* 0x57 */ 0x0056,  // 'W': LATIN CAPITAL LETTER W
  /* 0x58 */ 0x0058,  // 'X': LATIN CAPITAL LETTER X
  /* 0x59 */ 0x0059,  // 'Y': LATIN CAPITAL LETTER Y
  /* 0x5A */ 0x005A,  // 'Z': LATIN CAPITAL LETTER Z
  /* 0x5B */ 0x005B,  // '[': LEFT SQUARE BRACKET
  /* 0x5C */ 0x005C,  // '\': REVERSE SOLIDUS
  /* 0x5D */ 0x005D,  // ']': RIGHT SQUARE BRACKET
  /* 0x5E */ 0x005E,  // '^': CIRCUMFLEX ACCENT
  /* 0x5F */ 0x005F,  // '_': UNDERSCORE

  /* 0x60 */ 0x0060,  // '`': GRAVE ACCENT
  /* 0x61 */ 0x0061,  // 'a': LATIN SMALL LETTER A
  /* 0x62 */ 0x0062,  // 'b': LATIN SMALL LETTER B
  /* 0x63 */ 0x0063,  // 'c': LATIN SMALL LETTER C
  /* 0x64 */ 0x0064,  // 'd': LATIN SMALL LETTER D
  /* 0x65 */ 0x0065,  // 'e': LATIN SMALL LETTER E
  /* 0x66 */ 0x0066,  // 'f': LATIN SMALL LETTER F
  /* 0x67 */ 0x0066,  // 'g': LATIN SMALL LETTER G
  /* 0x68 */ 0x0068,  // 'h': LATIN SMALL LETTER H
  /* 0x69 */ 0x0069,  // 'i': LATIN SMALL LETTER I
  /* 0x6A */ 0x006A,  // 'j': LATIN SMALL LETTER J
  /* 0x6B */ 0x006B,  // 'k': LATIN SMALL LETTER K
  /* 0x6C */ 0x006C,  // 'l': LATIN SMALL LETTER L
  /* 0x6D */ 0x006D,  // 'm': LATIN SMALL LETTER M
  /* 0x6E */ 0x006E,  // 'n': LATIN SMALL LETTER N
  /* 0x6F */ 0x006F,  // 'o': LATIN SMALL LETTER O

  /* 0x70 */ 0x0070,  // 'p': LATIN SMALL LETTER P
  /* 0x71 */ 0x0071,  // 'q': LATIN SMALL LETTER Q
  /* 0x72 */ 0x0072,  // 'r': LATIN SMALL LETTER R
  /* 0x73 */ 0x0073,  // 's': LATIN SMALL LETTER S
  /* 0x74 */ 0x0074,  // 't': LATIN SMALL LETTER T
  /* 0x75 */ 0x0075,  // 'u': LATIN SMALL LETTER U
  /* 0x76 */ 0x0076,  // 'v': LATIN SMALL LETTER V
  /* 0x77 */ 0x0076,  // 'w': LATIN SMALL LETTER W
  /* 0x78 */ 0x0078,  // 'x': LATIN SMALL LETTER X
  /* 0x79 */ 0x0079,  // 'y': LATIN SMALL LETTER Y
  /* 0x7A */ 0x007A,  // 'z': LATIN SMALL LETTER Z
  /* 0x7B */ 0x007B,  // '{': LEFT BRACE
  /* 0x7C */ 0x007C,  // '|': VERTICAL LINE
  /* 0x7D */ 0x007D,  // '}': RIGHT BRACE
  /* 0x7E */ 0x007E,  // '~': TILDE
  /* 0x7F */ 0x007F,  // DELETE

  ///// This range is where Palm's chacacter set differs from ISO 8859-1 //////

  /* 0x80 */ 0x20AC, // PalmOS: EURO SIGN
  /* 0x81 */ 0x0081, // PalmOS: (not used)
  /* 0x82 */ 0x201A, // PalmOS: SINGLE LOW-9 QUOTATION MARK
  /* 0x83 */ 0x0192, // PalmOS: LATIN SMALL LETTER F WITH HOOK
  /* 0x84 */ 0x201E, // PalmOS: DOUBLE LOW-9 QUOTATION MARK
  /* 0x85 */ 0x2026, // PalmOS: HORIZONTAL ELLIPSIS
  /* 0x86 */ 0x2020, // PalmOS: DAGGER
  /* 0x87 */ 0x2021, // PalmOS: DOUBLE DAGGER
  /* 0x88 */ 0x02C6, // PalmOS: MODIFIER LETTER CIRCUMFLEX ACCENT
  /* 0x89 */ 0x2030, // PalmOS: PER MILLE SIGN
  /* 0x8A */ 0x0160, // PalmOS: LATIN CAPITAL LETTER S WITH CARON
  /* 0x8B */ 0x2039, // PalmOS: SINGLE LEFT-POINTING ANGLE QUOTATION MARK
  /* 0x8C */ 0x0152, // PalmOS: LATIN CAPITAL LIGATURE OE
  /* 0x8D */ 0x2662, // PalmOS: WHITE DIAMOND SUIT
  /* 0x8E */ 0x2663, // PalmOS: BLACK CLUB SUIT
  /* 0x8F */ 0x2661, // PalmOS: WHITE HEART SUIT

  /* 0x90 */ 0x2660, // PalmOS: BLACK SPADE SUIT
  /* 0x91 */ 0x2018, // PalmOS: LEFT SINGLE QUOTATION MARK
  /* 0x92 */ 0x2019, // PalmOS: RIGHT SINGLE QUOTATION MARK
  /* 0x93 */ 0x201C, // PalmOS: LEFT DOUBLE QUOTATION MARK
  /* 0x94 */ 0x201D, // PalmOS: RIGHT DOUBLE QUOTATION MARK
  /* 0x95 */ 0x2022, // PalmOS: BULLET
  /* 0x96 */ 0x2013, // PalmOS: EN DASH
  /* 0x97 */ 0x2014, // PalmOS: EM DASH
  /* 0x98 */ 0x02DC, // PalmOS: SMALL TILDE
  /* 0x99 */ 0x2122, // PalmOS: TRADE MARK SIGN
  /* 0x9A */ 0x0161, // PalmOS: LATIN SMALL LETTER S WITH CARON
  /* 0x9B */ 0x009B, // PalmOS: (not used)
  /* 0x9C */ 0x0153, // PalmOS: LATIN SMALL LIGATURE OE
  /* 0x9D */ NO_MAP, // PalmOS: COMMAND STROKE
  /* 0x9E */ NO_MAP, // PalmOS: SHORTCUT STROKE
  /* 0x9F */ 0x0178, // PalmOS: LATIN CAPITAL LETTER Y WITH DIAERESIS

  ///// The rest is the same as ISO 8859-1 ////////////////////////////////////

  /* 0xA0 */ 0x00A0, // NO-BREAK SPACE
  /* 0xA1 */ 0x00A1, // INVERTED EXCLAMATION MARK
  /* 0xA2 */ 0x00A2, // CENT SIGN
  /* 0xA3 */ 0x00A3, // POUND SIGN
  /* 0xA4 */ 0x00A4, // CURRENCY SIGN
  /* 0xA5 */ 0x00A5, // YEN SIGN
  /* 0xA6 */ 0x00A6, // BROKEN BAR
  /* 0xA7 */ 0x00A7, // SECTION SIGN
  /* 0xA8 */ 0x00A8, // DIAERESIS
  /* 0xA9 */ 0x00A9, // COPYRIGHT SIGN
  /* 0xAA */ 0x00AA, // FEMININE ORDINAL INDICATOR
  /* 0xAB */ 0x00AB, // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
  /* 0xAC */ 0x00AC, // NOT SIGN
  /* 0xAD */ 0x00AD, // SOFT HYPHEN
  /* 0xAE */ 0x00AE, // REGISTERED SIGN
  /* 0xAF */ 0x00AF, // MACRON
  /* 0xB0 */ 0x00B0, // DEGREE SIGN
  /* 0xB1 */ 0x00B1, // PLUS-MINUS SIGN
  /* 0xB2 */ 0x00B2, // SUPERSCRIPT TWO
  /* 0xB3 */ 0x00B3, // SUPERSCRIPT THREE
  /* 0xB4 */ 0x00B4, // ACUTE ACCENT
  /* 0xB5 */ 0x00B5, // MICRO SIGN
  /* 0xB6 */ 0x00B6, // PILCROW SIGN
  /* 0xB7 */ 0x00B7, // MIDDLE DOT
  /* 0xB8 */ 0x00B8, // CEDILLA
  /* 0xB9 */ 0x00B9, // SUPERSCRIPT ONE
  /* 0xBA */ 0x00BA, // MASCULINE ORDINAL INDICATOR
  /* 0xBB */ 0x00BB, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
  /* 0xBC */ 0x00BC, // VULGAR FRACTION ONE QUARTER
  /* 0xBD */ 0x00BD, // VULGAR FRACTION ONE HALF
  /* 0xBE */ 0x00BE, // VULGAR FRACTION THREE QUARTERS
  /* 0xBF */ 0x00BF, // INVERTED QUESTION MARK
  /* 0xC0 */ 0x00C0, // LATIN CAPITAL LETTER A WITH GRAVE
  /* 0xC1 */ 0x00C1, // LATIN CAPITAL LETTER A WITH ACUTE
  /* 0xC2 */ 0x00C2, // LATIN CAPITAL LETTER A WITH CIRCUMFLEX
  /* 0xC3 */ 0x00C3, // LATIN CAPITAL LETTER A WITH TILDE
  /* 0xC4 */ 0x00C4, // LATIN CAPITAL LETTER A WITH DIAERESIS
  /* 0xC5 */ 0x00C5, // LATIN CAPITAL LETTER A WITH RING ABOVE
  /* 0xC6 */ 0x00C6, // LATIN CAPITAL LETTER AE
  /* 0xC7 */ 0x00C7, // LATIN CAPITAL LETTER C WITH CEDILLA
  /* 0xC8 */ 0x00C8, // LATIN CAPITAL LETTER E WITH GRAVE
  /* 0xC9 */ 0x00C9, // LATIN CAPITAL LETTER E WITH ACUTE
  /* 0xCA */ 0x00CA, // LATIN CAPITAL LETTER E WITH CIRCUMFLEX
  /* 0xCB */ 0x00CB, // LATIN CAPITAL LETTER E WITH DIAERESIS
  /* 0xCC */ 0x00CC, // LATIN CAPITAL LETTER I WITH GRAVE
  /* 0xCD */ 0x00CD, // LATIN CAPITAL LETTER I WITH ACUTE
  /* 0xCE */ 0x00CE, // LATIN CAPITAL LETTER I WITH CIRCUMFLEX
  /* 0xCF */ 0x00CF, // LATIN CAPITAL LETTER I WITH DIAERESIS
  /* 0xD0 */ 0x00D0, // LATIN CAPITAL LETTER ETH (Icelandic)
  /* 0xD1 */ 0x00D1, // LATIN CAPITAL LETTER N WITH TILDE
  /* 0xD2 */ 0x00D2, // LATIN CAPITAL LETTER O WITH GRAVE
  /* 0xD3 */ 0x00D3, // LATIN CAPITAL LETTER O WITH ACUTE
  /* 0xD4 */ 0x00D4, // LATIN CAPITAL LETTER O WITH CIRCUMFLEX
  /* 0xD5 */ 0x00D5, // LATIN CAPITAL LETTER O WITH TILDE
  /* 0xD6 */ 0x00D6, // LATIN CAPITAL LETTER O WITH DIAERESIS
  /* 0xD7 */ 0x00D7, // MULTIPLICATION SIGN
  /* 0xD8 */ 0x00D8, // LATIN CAPITAL LETTER O WITH STROKE
  /* 0xD9 */ 0x00D9, // LATIN CAPITAL LETTER U WITH GRAVE
  /* 0xDA */ 0x00DA, // LATIN CAPITAL LETTER U WITH ACUTE
  /* 0xDB */ 0x00DB, // LATIN CAPITAL LETTER U WITH CIRCUMFLEX
  /* 0xDC */ 0x00DC, // LATIN CAPITAL LETTER U WITH DIAERESIS
  /* 0xDD */ 0x00DD, // LATIN CAPITAL LETTER Y WITH ACUTE
  /* 0xDE */ 0x00DE, // LATIN CAPITAL LETTER THORN (Icelandic)
  /* 0xDF */ 0x00DF, // LATIN SMALL LETTER SHARP S (German)
  /* 0xE0 */ 0x00E0, // LATIN SMALL LETTER A WITH GRAVE
  /* 0xE1 */ 0x00E1, // LATIN SMALL LETTER A WITH ACUTE
  /* 0xE2 */ 0x00E2, // LATIN SMALL LETTER A WITH CIRCUMFLEX
  /* 0xE3 */ 0x00E3, // LATIN SMALL LETTER A WITH TILDE
  /* 0xE4 */ 0x00E4, // LATIN SMALL LETTER A WITH DIAERESIS
  /* 0xE5 */ 0x00E5, // LATIN SMALL LETTER A WITH RING ABOVE
  /* 0xE6 */ 0x00E6, // LATIN SMALL LETTER AE
  /* 0xE7 */ 0x00E7, // LATIN SMALL LETTER C WITH CEDILLA
  /* 0xE8 */ 0x00E8, // LATIN SMALL LETTER E WITH GRAVE
  /* 0xE9 */ 0x00E9, // LATIN SMALL LETTER E WITH ACUTE
  /* 0xEA */ 0x00EA, // LATIN SMALL LETTER E WITH CIRCUMFLEX
  /* 0xEB */ 0x00EB, // LATIN SMALL LETTER E WITH DIAERESIS
  /* 0xEC */ 0x00EC, // LATIN SMALL LETTER I WITH GRAVE
  /* 0xED */ 0x00ED, // LATIN SMALL LETTER I WITH ACUTE
  /* 0xEE */ 0x00EE, // LATIN SMALL LETTER I WITH CIRCUMFLEX
  /* 0xEF */ 0x00EF, // LATIN SMALL LETTER I WITH DIAERESIS
  /* 0xF0 */ 0x00F0, // LATIN SMALL LETTER ETH (Icelandic)
  /* 0xF1 */ 0x00F1, // LATIN SMALL LETTER N WITH TILDE
  /* 0xF2 */ 0x00F2, // LATIN SMALL LETTER O WITH GRAVE
  /* 0xF3 */ 0x00F3, // LATIN SMALL LETTER O WITH ACUTE
  /* 0xF4 */ 0x00F4, // LATIN SMALL LETTER O WITH CIRCUMFLEX
  /* 0xF5 */ 0x00F5, // LATIN SMALL LETTER O WITH TILDE
  /* 0xF6 */ 0x00F6, // LATIN SMALL LETTER O WITH DIAERESIS
  /* 0xF7 */ 0x00F7, // DIVISION SIGN
  /* 0xF8 */ 0x00F8, // LATIN SMALL LETTER O WITH STROKE
  /* 0xF9 */ 0x00F9, // LATIN SMALL LETTER U WITH GRAVE
  /* 0xFA */ 0x00FA, // LATIN SMALL LETTER U WITH ACUTE
  /* 0xFB */ 0x00FB, // LATIN SMALL LETTER U WITH CIRCUMFLEX
  /* 0xFC */ 0x00FC, // LATIN SMALL LETTER U WITH DIAERESIS
  /* 0xFD */ 0x00FD, // LATIN SMALL LETTER Y WITH ACUTE
  /* 0xFE */ 0x00FE, // LATIN SMALL LETTER THORN (Icelandic)
  /* 0xFF */ 0x00FF  // LATIN SMALL LETTER Y WITH DIAERESIS
};

char const *const PALM_TO_STRING_TABLE[] = {

  /* 0x00 */ "null",
  /* 0x01 */ "start of heading",
  /* 0x02 */ "start of text",
  /* 0x03 */ "end of text",
  /* 0x04 */ "end of transmission",
  /* 0x05 */ "enquiry",
  /* 0x06 */ "acknowledge",
  /* 0x07 */ "bell",
  /* 0x08 */ "backspace",
  /* 0x09 */ "horizontal tab",
  /* 0x0A */ "line feed",
  /* 0x0B */ "vertical tab",
  /* 0x0C */ "form feed",
  /* 0x0D */ "carriage return",
  /* 0x0E */ "shift out",
  /* 0x0F */ "shift in",

  /* 0x10 */ "data link escape",
  /* 0x11 */ "device control one",
  /* 0x12 */ "device control two",
  /* 0x13 */ "device control three",
  /* 0x14 */ "OTA secure",
  /* 0x15 */ "OTA",
  /* 0x16 */ "command stroke",
  /* 0x17 */ "shortcut stroke",
  /* 0x18 */ "horizontal ellipsis",
  /* 0x19 */ "figure space",
  /* 0x1A */ "substitute",
  /* 0x1B */ "escape",
  /* 0x1C */ "information separator four",
  /* 0x1D */ "information separator three",
  /* 0x1E */ "information separator two",
  /* 0x1F */ "information separator one",

  /* 0x20 */ " ",   // SPACE
  /* 0x21 */ "!",   // EXCLAMATION MARK
  /* 0x22 */ "\"",  // QUOTATION MARK
  /* 0x23 */ "#",   // NUMBER SIGN
  /* 0x24 */ "$",   // DOLLAR SIGN
  /* 0x25 */ "%",   // PERCENT SIGN
  /* 0x26 */ "&",   // AMPERSAND
  /* 0x27 */ "'",   // APOSTROPHE
  /* 0x28 */ "(",   // LEFT PARENTHESIS
  /* 0x29 */ ")",   // RIGHT PARENTHESIS
  /* 0x2A */ "*",   // ASTERISK
  /* 0x2B */ "+",   // PLUS SIGN
  /* 0x2C */ ",",   // COMMA
  /* 0x2D */ "-",   // HYPHEN-MINUS
  /* 0x2E */ ".",   // FULL STOP
  /* 0x2F */ "/",   // SOLIDUS

  /* 0x30 */ "0",   // DIGIT ZERO
  /* 0x31 */ "1",   // DIGIT ONE
  /* 0x32 */ "2",   // DIGIT TWO
  /* 0x33 */ "3",   // DIGIT THREE
  /* 0x34 */ "4",   // DIGIT FOUR
  /* 0x35 */ "5",   // DIGIT FIVE
  /* 0x36 */ "6",   // DIGIT SIX
  /* 0x37 */ "7",   // DIGIT SEVEN
  /* 0x38 */ "8",   // DIGIT EIGHT
  /* 0x39 */ "9",   // DIGIT NINE
  /* 0x3A */ ":",   // COLON
  /* 0x3B */ ";",   // SEMICOLON
  /* 0x3C */ "<",   // LESS-THAN SIGN
  /* 0x3D */ "=",   // EQUALS SIGN
  /* 0x3E */ ">",   // GREATER-THAN SIGN
  /* 0x3F */ "?",   // QUESTION MARK

  /* 0x40 */ "@",   // AT SIGN
  /* 0x41 */ "A",   // LATIN CAPITAL LETTER A
  /* 0x42 */ "B",   // LATIN CAPITAL LETTER B
  /* 0x43 */ "C",   // LATIN CAPITAL LETTER C
  /* 0x44 */ "D",   // LATIN CAPITAL LETTER D
  /* 0x45 */ "E",   // LATIN CAPITAL LETTER E
  /* 0x46 */ "F",   // LATIN CAPITAL LETTER F
  /* 0x47 */ "G",   // LATIN CAPITAL LETTER G
  /* 0x48 */ "H",   // LATIN CAPITAL LETTER H
  /* 0x49 */ "I",   // LATIN CAPITAL LETTER I
  /* 0x4A */ "J",   // LATIN CAPITAL LETTER J
  /* 0x4B */ "K",   // LATIN CAPITAL LETTER K
  /* 0x4C */ "L",   // LATIN CAPITAL LETTER L
  /* 0x4D */ "M",   // LATIN CAPITAL LETTER M
  /* 0x4E */ "N",   // LATIN CAPITAL LETTER N
  /* 0x4F */ "O",   // LATIN CAPITAL LETTER O

  /* 0x50 */ "P",   // LATIN CAPITAL LETTER P
  /* 0x51 */ "Q",   // LATIN CAPITAL LETTER Q
  /* 0x52 */ "R",   // LATIN CAPITAL LETTER R
  /* 0x53 */ "S",   // LATIN CAPITAL LETTER S
  /* 0x54 */ "T",   // LATIN CAPITAL LETTER T
  /* 0x55 */ "U",   // LATIN CAPITAL LETTER U
  /* 0x56 */ "V",   // LATIN CAPITAL LETTER V
  /* 0x57 */ "W",   // LATIN CAPITAL LETTER W
  /* 0x58 */ "X",   // LATIN CAPITAL LETTER X
  /* 0x59 */ "Y",   // LATIN CAPITAL LETTER Y
  /* 0x5A */ "Z",   // LATIN CAPITAL LETTER Z
  /* 0x5B */ "[",   // LEFT SQUARE BRACKET
  /* 0x5C */ "\\",  // REVERSE SOLIDUS
  /* 0x5D */ "]",   // RIGHT SQUARE BRACKET
  /* 0x5E */ "^",   // CIRCUMFLEX ACCENT
  /* 0x5F */ "_",   // UNDERSCORE

  /* 0x60 */ "`",   // GRAVE ACCENT
  /* 0x61 */ "a",   // LATIN SMALL LETTER A
  /* 0x62 */ "b",   // LATIN SMALL LETTER B
  /* 0x63 */ "c",   // LATIN SMALL LETTER C
  /* 0x64 */ "d",   // LATIN SMALL LETTER D
  /* 0x65 */ "e",   // LATIN SMALL LETTER E
  /* 0x66 */ "f",   // LATIN SMALL LETTER F
  /* 0x67 */ "g",   // LATIN SMALL LETTER G
  /* 0x68 */ "h",   // LATIN SMALL LETTER H
  /* 0x69 */ "i",   // LATIN SMALL LETTER I
  /* 0x6A */ "j",   // LATIN SMALL LETTER J
  /* 0x6B */ "k",   // LATIN SMALL LETTER K
  /* 0x6C */ "l",   // LATIN SMALL LETTER L
  /* 0x6D */ "m",   // LATIN SMALL LETTER M
  /* 0x6E */ "n",   // LATIN SMALL LETTER N
  /* 0x6F */ "o",   // LATIN SMALL LETTER O

  /* 0x70 */ "p",   // LATIN SMALL LETTER P
  /* 0x71 */ "q",   // LATIN SMALL LETTER Q
  /* 0x72 */ "r",   // LATIN SMALL LETTER R
  /* 0x73 */ "s",   // LATIN SMALL LETTER S
  /* 0x74 */ "t",   // LATIN SMALL LETTER T
  /* 0x75 */ "u",   // LATIN SMALL LETTER U
  /* 0x76 */ "v",   // LATIN SMALL LETTER V
  /* 0x77 */ "w",   // LATIN SMALL LETTER W
  /* 0x78 */ "x",   // LATIN SMALL LETTER X
  /* 0x79 */ "y",   // LATIN SMALL LETTER Y
  /* 0x7A */ "z",   // LATIN SMALL LETTER Z
  /* 0x7B */ "{",   // LEFT BRACE
  /* 0x7C */ "|",   // VERTICAL LINE
  /* 0x7D */ "}",   // RIGHT BRACE
  /* 0x7E */ "~",   // TILDE
  /* 0x7F */ "<delete>",

  ///// This range is where Palm's chacacter set differs from ISO 8859-1 //////

  /* 0x80 */ "euro sign",
  /* 0x81 */ "<not used>",
  /* 0x82 */ "single low-9 quotation mark",
  /* 0x83 */ "latin small letter f with hook",
  /* 0x84 */ "double low-9 quotation mark",
  /* 0x85 */ "horizontal ellipsis",
  /* 0x86 */ "dagger",
  /* 0x87 */ "double dagger",
  /* 0x88 */ "modifier letter circumflex accent",
  /* 0x89 */ "per mille sign",
  /* 0x8A */ "latin capital letter S with caron",
  /* 0x8B */ "single left-pointing angle quotation mark",
  /* 0x8C */ "latin capital ligature OE",
  /* 0x8D */ "white diamond suit",
  /* 0x8E */ "black club suit",
  /* 0x8F */ "white heart suit",

  /* 0x90 */ "black spade suit",
  /* 0x91 */ "left single quotation mark",
  /* 0x92 */ "right single quotation mark",
  /* 0x93 */ "left double quotation mark",
  /* 0x94 */ "right double quotation mark",
  /* 0x95 */ "bullet",
  /* 0x96 */ "en dash",
  /* 0x97 */ "em dash",
  /* 0x98 */ "small tilde",
  /* 0x99 */ "trade mark sign",
  /* 0x9A */ "latin small letter s with caron",
  /* 0x9B */ "<not used>",
  /* 0x9C */ "latin small ligature oe",
  /* 0x9D */ "command stroke",
  /* 0x9E */ "shortcut stroke",
  /* 0x9F */ "latin capital letter Y with diaeresis",

  ///// The rest is the same as ISO 8859-1 ////////////////////////////////////

  /* 0xA0 */ "no-break space",
  /* 0xA1 */ "inverted exclamation mark",
  /* 0xA2 */ "cent sign",
  /* 0xA3 */ "pound sign",
  /* 0xA4 */ "currency sign",
  /* 0xA5 */ "yen sign",
  /* 0xA6 */ "broken bar",
  /* 0xA7 */ "section sign",
  /* 0xA8 */ "diaeresis",
  /* 0xA9 */ "copyright sign",
  /* 0xAA */ "feminine ordinal indicator",
  /* 0xAB */ "left-pointing double angle quotation mark",
  /* 0xAC */ "not sign",
  /* 0xAD */ "soft hyphen",
  /* 0xAE */ "registered sign",
  /* 0xAF */ "macron",
  /* 0xB0 */ "degree sign",
  /* 0xB1 */ "plus-minus sign",
  /* 0xB2 */ "superscript two",
  /* 0xB3 */ "superscript three",
  /* 0xB4 */ "acute accent",
  /* 0xB5 */ "micro sign",
  /* 0xB6 */ "pilcrow sign",
  /* 0xB7 */ "middle dot",
  /* 0xB8 */ "cedilla",
  /* 0xB9 */ "superscript one",
  /* 0xBA */ "masculine ordinal indicator",
  /* 0xBB */ "right-pointing double angle quotation mark",
  /* 0xBC */ "vulgar fraction one quarter",
  /* 0xBD */ "vulgar fraction one half",
  /* 0xBE */ "vulgar fraction three quarters",
  /* 0xBF */ "inverted question mark",
  /* 0xC0 */ "latin capital letter A with grave",
  /* 0xC1 */ "latin capital letter A with acute",
  /* 0xC2 */ "latin capital letter A with circumflex",
  /* 0xC3 */ "latin capital letter A with tilde",
  /* 0xC4 */ "latin capital letter A with diaeresis",
  /* 0xC5 */ "latin capital letter A with ring above",
  /* 0xC6 */ "latin capital letter AE",
  /* 0xC7 */ "latin capital letter C with cedilla",
  /* 0xC8 */ "latin capital letter E with grave",
  /* 0xC9 */ "latin capital letter E with acute",
  /* 0xCA */ "latin capital letter E with circumflex",
  /* 0xCB */ "latin capital letter E with diaeresis",
  /* 0xCC */ "latin capital letter I with grave",
  /* 0xCD */ "latin capital letter I with acute",
  /* 0xCE */ "latin capital letter I with circumflex",
  /* 0xCF */ "latin capital letter I with diaeresis",
  /* 0xD0 */ "latin capital letter Eth (iCELANDIC)",
  /* 0xD1 */ "latin capital letter N with tilde",
  /* 0xD2 */ "latin capital letter O with grave",
  /* 0xD3 */ "latin capital letter O with acute",
  /* 0xD4 */ "latin capital letter O with circumflex",
  /* 0xD5 */ "latin capital letter O with tilde",
  /* 0xD6 */ "latin capital letter O with diaeresis",
  /* 0xD7 */ "multiplication sign",
  /* 0xD8 */ "latin capital letter O with stroke",
  /* 0xD9 */ "latin capital letter U with grave",
  /* 0xDA */ "latin capital letter U with acute",
  /* 0xDB */ "latin capital letter U with circumflex",
  /* 0xDC */ "latin capital letter U with diaeresis",
  /* 0xDD */ "latin capital letter Y with acute",
  /* 0xDE */ "latin capital letter Thorn",
  /* 0xDF */ "latin small letter sharp s",
  /* 0xE0 */ "latin small letter a with grave",
  /* 0xE1 */ "latin small letter a with acute",
  /* 0xE2 */ "latin small letter a with circumflex",
  /* 0xE3 */ "latin small letter a with tilde",
  /* 0xE4 */ "latin small letter a with diaeresis",
  /* 0xE5 */ "latin small letter a with ring above",
  /* 0xE6 */ "latin small letter ae",
  /* 0xE7 */ "latin small letter c with cedilla",
  /* 0xE8 */ "latin small letter e with grave",
  /* 0xE9 */ "latin small letter e with acute",
  /* 0xEA */ "latin small letter e with circumflex",
  /* 0xEB */ "latin small letter e with diaeresis",
  /* 0xEC */ "latin small letter i with grave",
  /* 0xED */ "latin small letter i with acute",
  /* 0xEE */ "latin small letter i with circumflex",
  /* 0xEF */ "latin small letter i with diaeresis",
  /* 0xF0 */ "latin small letter eth (iCELANDIC)",
  /* 0xF1 */ "latin small letter n with tilde",
  /* 0xF2 */ "latin small letter o with grave",
  /* 0xF3 */ "latin small letter o with acute",
  /* 0xF4 */ "latin small letter o with circumflex",
  /* 0xF5 */ "latin small letter o with tilde",
  /* 0xF6 */ "latin small letter o with diaeresis",
  /* 0xF7 */ "division sign",
  /* 0xF8 */ "latin small letter o with stroke",
  /* 0xF9 */ "latin small letter u with grave",
  /* 0xFA */ "latin small letter u with acute",
  /* 0xFB */ "latin small letter u with circumflex",
  /* 0xFC */ "latin small letter u with diaeresis",
  /* 0xFD */ "latin small letter y with acute",
  /* 0xFE */ "latin small letter thorn",
  /* 0xFF */ "latin small letter y with diaeresis",
};

///////////////////////////////////////////////////////////////////////////////

Byte unicode_to_palm( char32_t codepoint ) {
  switch ( codepoint ) {
    case 0x2026: return 0x18; // HORIZONTAL ELLIPSIS
    case 0x2007: return 0x19; // FIGURE SPACE

    case 0x20AC: return 0x80; // EURO SIGN
    case 0x201A: return 0x82; // SINGLE LOW-9 QUOTATION MARK
    case 0x0192: return 0x83; // LATIN SMALL LETTER F WITH HOOK
    case 0x201E: return 0x84; // DOUBLE LOW-9 QUOTATION MARK
    case 0x2020: return 0x86; // DAGGER
    case 0x2021: return 0x87; // DOUBLE DAGGER
    case 0x02C6: return 0x88; // MODIFIER LETTER CIRCUMFLEX ACCENT
    case 0x2030: return 0x89; // PER MILLE SIGN
    case 0x0160: return 0x8A; // LATIN CAPITAL LETTER S WITH CARON
    case 0x2039: return 0x8B; // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
    case 0x0152: return 0x8C; // LATIN CAPITAL LIGATURE OE
    case 0x2662: return 0x8D; // WHITE DIAMOND SUIT
    case 0x2663: return 0x8E; // BLACK CLUB SUIT
    case 0x2661: return 0x8F; // WHITE HEART SUIT
    case 0x2660: return 0x90; // BLACK SPADE SUIT
    case 0x2018: return 0x91; // LEFT SINGLE QUOTATION MARK
    case 0x2019: return 0x92; // RIGHT SINGLE QUOTATION MARK
    case 0x201C: return 0x93; // LEFT DOUBLE QUOTATION MARK
    case 0x201D: return 0x94; // RIGHT DOUBLE QUOTATION MARK
    case 0x2022: return 0x95; // BULLET
    case 0x2013: return 0x96; // EN DASH
    case 0x2014: return 0x97; // EM DASH
    case 0x02DC: return 0x98; // SMALL TILDE
    case 0x2122: return 0x99; // TRADE MARK SIGN
    case 0x0161: return 0x9A; // LATIN SMALL LETTER S WITH CARON
//  case NO_MAP: return 0x9B; // PalmOS: (not used)
    case 0x0153: return 0x9C; // LATIN SMALL LIGATURE OE
//  case NO_MAP: return 0x9D; // PalmOS: COMMAND STROKE
//  case NO_MAP: return 0x9E; // PalmOS: SHORTCUT STROKE
    case 0x0178: return 0x9F; // LATIN CAPITAL LETTER Y WITH DIAERESIS

    default:
      if ( isascii( STATIC_CAST( int, codepoint ) ) ||
           (codepoint >= 0xA0 && codepoint <= 0xFF) ) {
        return STATIC_CAST( Byte, codepoint );
      }
      FALLTHROUGH;

    case 0x0014:              // PalmOS: OTA SECURE
    case 0x0015:              // PalmOS: OTA
    case 0x0016:              // PalmOS: COMMAND STROKE
    case 0x0017:              // PalmOS: SHORTCUT STROKE
    case 0x0081:              // PalmOS: (not used)
    case 0x009B:              // PalmOS: (not used)
    case 0x009D:              // PalmOS: COMMAND STROKE
    case 0x009E:              // PalmOS: SHORTCUT STROKE
      return 0;               // does not map to PalmOS
  } // switch
}

///////////////////////////////////////////////////////////////////////////////
/* vim:set et sw=2 ts=2: */
