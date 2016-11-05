/*
   emu7470 emulator engine for a HP7470A plotter for use
   with the pyILPER software

   Derived from HP2XX
   Copyright (c) 1991 - 1994 Heinz W. Werntges
   Parts Copyright (c) 1999 - 2001 Martin Kroeker
   em7470 (C) 2016 Joachim Siebold

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*/
/** charset2.h          HP-GL character set 4, Spanish/Latin American
 **                              MK February 1999
 **/
/**
 ** This file defines a standard character set by elementary
 ** "draw" & "move" commands. The format is a very compact one from
 ** the old days where every byte was still appreciated.
 **
 ** A font or character set is an array of strings. Each character
 ** corresponds to one of these strings, which is addressed by its ASCII code.
 **
 ** A character is a (NULL-terminated) string of bytes. Each byte
 ** codes for a draw or move action according to the code below:
 **
 **	Bit:	7 6 5 4 3 2 1 0
 **		p x x x y y y y
 **
 **	p:	Plot flag. If set, "draw to" new point, else "move to" it.
 **	xxx:	3-bit unsigned integer  (0...7). X coordinate of new point.
 **	yyyy:	4-bit unsigned integer (0..15). Y coordinate of new point.
 **
 ** The baseline is y = 4 instead of y = 0, so characters with parts
 ** below it can be drawn properly without a need for sign bits.
 ** Function "code_to_ucoord()" transforms these coordinates into
 ** actual user coordinates.
 **
 ** Example:	code for character 'L': "\032\224\324" translates to:
 **		moveto(1,10); drawto(1,4); drawto(5,4);
 **
 ** From the example you can conclude that the font below essentially is
 ** defined on a 5x7 grid:
 **
 **	  	0 1 2 3 4 5 6 7
 **	15	. . . . . . . .		. : unused
 **	14	. . . . . . . .		* : always used
 **	13	. . . . . . . .		o : sometimes used
 **	12	. . . . . . . .
 **	11	. . . . . . . .
 **	10	o * * * * * . .
 **	 9	o * * * * * . .
 **	 8	o * * * * * . .
 **	 7	o * * * * * . .
 **	 6	o * * * * * . .
 **	 5	o * * * * * . .
 **	 4	o * * * * * . .
 **	 3	o o o o o o . .
 **	 2	o o o o o o . .
 **	 1	o o o o o o . .
 **	 0	o o o o o o . .
 **/


/**
 ** The following array of strings contains the basic character set (set 0).
 **
 ** NOTE: A nice way to add a new charset would be, e. g., to introduce a
 ** ``charset1[]'' as the "alternate" charset and implement the HP-GL
 ** commands needed for switching from one to the other.
 **/

char *charset4[128] = {
	/* 0x00 ... 0x1f        */

/**
 ** Some control codes are valid in HPGL. These are handled elsewhere
 ** in a font-independent manner, so following codes are dummies:
 **/
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",

/**
 ** Unfortunately, some compilers do not process \xNN properly,
 ** so I changed all hex codes (\xNN) into octal codes (\NNN),
 ** thereby losing readability but gaining portability.
 **/

	/* 0x20 ... 0x2f        */
	"",
	"\064\265\066\272",
	"\051\252\111\312",
	"\072\271\070\267\247\226\225\244\304\325",	/*inverted question mark */
	"\064\272\131\251\230\247\307\326\305\225",
	"\024\332\051\250\270\271\251\066\265\305\306\266",
	"\124\230\231\252\271\270\226\225\244\264\326",
	"\052\315",		/* (BS) acute */
	"\132\270\266\324",
	"\024\266\270\232",
	"\005\351\145\211\072\264",
	"\065\271\027\327",
	"\064\244\245\265\263\242",
	"\027\327",
	"\064\244\245\265\264",
	"\352",

	/* 0x30 ... 0x3f        */
/*
"\025\244\304\325\331\312\252\231\225\331", ** Zero including `/' **
*/
	"\025\244\304\325\331\312\252\231\225",
	"\044\304\064\272\251",
	"\031\252\312\331\330\225\224\324",
	"\025\244\304\325\326\307\267\332\232",
	"\112\227\226\326\107\304",
	"\132\232\230\310\327\325\304\244\225",
	"\132\272\230\225\244\304\325\326\307\227",
	"\032\332\331\226\224",
	"\107\330\331\312\252\231\230\247\307\326\325\304\244\225\226\247",
	"\044\264\326\331\312\252\231\230\247\327",
	"\047\250\270\267\247\045\265\264\244\245",
	"\046\247\267\266\246\064\244\245\265\263\242",
	"\112\227\304",
	"\030\330\026\326",
	"\032\307\224",
	"\031\252\312\331\330\307\267\266\065\264",

	/* 0x40 ... 0x4f        */
	"\103\243\224\230\252\312\331\326\305\266\267\310\330",
	"\024\231\252\312\331\324\026\326",
	"\024\232\312\331\330\307\227\024\304\325\326\307",
	"\125\304\244\225\231\252\312\331",
	"\024\232\312\331\325\304\224",
	"\124\224\232\332\027\307",
	"\024\232\332\027\307",
	//"\131\312\252\231\225\244\304\325\327\247",
	"\131\312\252\231\225\244\304\325\327\267",
	"\024\232\124\332\027\327",
	"\024\324\064\272\032\332",
	"\025\244\304\325\332\232",
	"\024\232\027\247\324\047\332",
	"\032\224\324",
	"\024\232\270\332\324",
	"\024\232\324\332",
	"\044\225\231\252\312\331\325\304\244",

	/* 0x50 ... 0x5f        */
	"\024\232\312\331\330\307\227",
	"\044\225\231\252\312\331\326\264\244\066\324",
	"\024\232\312\331\330\307\227\247\324",
	"\025\244\304\325\326\307\247\230\231\252\312\331",
	"\064\272\232\332",
	"\032\225\244\304\325\332",
	"\032\230\264\330\332",
	"\032\224\267\324\332",
	"\024\332\124\232",
	"\032\231\266\264\066\331\332",
	"\032\332\224\324",
	"\124\264\272\332",
	"\072\271\070\264",	/* inverted exclamation mark */
	"\024\264\272\232",
	"\030\272\330",		/* (BS) hat */
	"\023\323",		/* (BS) underline */

	/* 0x60 ... 0x6f        */
	"\053\310",
	"\124\244\225\227\250\310\304",
	"\024\304\325\327\310\250\052\244",
	"\125\304\264\245\247\270\310\327",
	"\112\304\244\225\227\250\310\104\324",
	"\026\306\327\310\250\227\225\244\324",
	"\064\271\312\332\047\307",
	"\022\262\303\310\250\227\225\244\304",
	"\032\224\030\270\307\304",
	"\072\271\050\270\264\044\304",
	"\072\271\050\270\263\242\222",
	"\024\232\104\226\310",
	"\052\272\264\044\304",
	"\024\230\027\250\267\264\067\310\327\324",
	"\024\230\027\250\270\307\304",
	"\044\225\227\250\270\307\305\264\244",

	/* 0x70 ... 0x7f        */
	"\022\230\270\307\305\264\224",
	"\104\244\225\227\250\310\302",
	"\030\224\026\270\310",
	"\110\250\227\246\266\305\264\224",
	"\052\244\304\030\310",
	"\030\225\244\304\310",
	"\030\226\264\326\330",
	"\030\225\244\265\267\065\304\325\330",
	"\030\324\024\330",
	"\022\326\330\030\226\264",
	"\030\310\224\304",
	"\013\254\333\374",	/* (BS) wide tilde (caps) */
	"\033\254\312\333",	/* (BS) tilde (caps)  */
	"\012\253\332\372",	/* (BS) wide tilde (lowercase) */
	"\032\253\311\332",	/* (BS) tilde  */
	""
};
