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
/** charset7.h                    HP-GL character set 7, Roman Extensions
 **                                MK February 1999
 **/
/**
 ** This file defines the HP Roman8 character set(#7) by elementary
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

char *charset7[128] = {
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
	"\024\231\252\312\331\324\026\326\055\312",	/* A grave */
	"\024\231\252\312\331\324\026\326\032\274\332",	/* A circumflex */
	"\124\224\232\332\027\307\055\312",	/*E grave */
	"\124\224\232\332\027\307\032\274\332",	/* E circumflex */
	"\124\224\232\332\027\307\054\253\114\313",	/* E diaresis */
	"\024\324\064\272\032\332\032\274\332",	/* I circumflex */
	"\024\324\064\272\032\332\054\253\114\313",	/* I diaresis */
	"\052\315",		/* acute */
	"\055\312",		/* grave */
	"\032\274\332",		/* circumflex */
	"\054\253\114\313",	/*diaresis */
	"\032\253\311\332",	/*tilde */
	"\032\225\244\304\325\332\055\312",	/*U grave */
	"\032\225\244\304\325\332\033\275\333",	/*U circumflex */
	"\131\332\312\270\265\244\224\225\246\304\325\050\310\047\307",	/*poundsign */

	/* 0x30 ... 0x3f        */
	"\033\333",		/*overbar */
	"\032\231\266\264\066\331\332\052\315",	/*Y acute */
	"\022\326\330\030\226\264\052\315",	/*y acute */
	"\050\251\272\311\310\267\250",	/* degree sign */
	"\125\304\244\225\231\252\312\331\064\243",	/*C cedilla */
	"\125\304\264\245\247\270\310\327\064\243",	/* c cedilla */
	"\024\232\324\332\033\254\312\333",	/* N tilde */
	"\024\230\027\250\270\307\304\032\253\311\332",	/* n tilde */
	"\072\271\070\264",	/*upsidedown exclam */
	"\072\271\070\267\247\226\225\244\304\325",	/*upsidedown questionmark */
	"\045\226\230\251\311\330\326\305\245\032\251\132\311\024\245\124\305",	/* box O */
	"\131\332\312\270\265\244\224\225\246\304\325\050\310",	/*singlebar poundsign */
	"\032\270\264\070\332\050\310\047\307",	/* yensign */
	"\111\272\251\250\270\307\306\304\263\243\244\105\265\246\250",	/* paragraph */
	"\131\332\312\270\265\244\224\225\050\310",	/* franc sign */
	"\125\304\264\245\247\270\310\327\063\271",	/* cent sign */

	/* 0x40 ... 0x4f        */
	"\124\244\225\227\250\310\304\051\272\311",	/* a circumflex */
	"\026\306\327\310\250\227\225\244\324\051\272\311",	/* e circumflex */
	"\044\225\227\250\270\307\305\264\244\051\272\311",	/* o circumflex */
	"\030\225\244\304\310\051\272\311",	/* u circumflex */
	"\124\244\225\227\250\310\304\051\313",	/* a acute */
	"\026\306\327\310\250\227\225\244\324\051\313",	/* e acute */
	"\044\225\227\250\270\307\305\264\244\051\313",	/* o acute */
	"\030\225\244\304\310\051\313",	/* u acute */
	"\124\244\225\227\250\310\304\053\311",	/* a grave */
	"\026\306\327\310\250\227\225\244\324\053\311",	/* e grave */
	"\044\225\227\250\270\307\305\264\244\053\311",	/* o grave */
	"\030\225\244\304\310\053\311",	/* u grave */
	"\124\244\225\227\250\310\304\052\251\112\311",	/* a diaresis */
	"\026\306\327\310\250\227\225\244\324\052\251\112\311",	/* e diaresis */
	"\044\225\227\250\270\307\305\264\244\052\251\112\311",	/* o diaresis */
	"\030\225\244\304\310\052\251\112\311",	/* u diaresis */

	/* 0x50 ... 0x5f        */
	"\024\231\252\312\331\324\026\326\072\253\274\313\272",	/* A ring */
	"\050\270\264\044\304\051\272\311",	/* i circumflex */
	"\025\244\304\325\331\312\252\231\225\331",	/* slashed zero */
	"\132\272\264\324\127\247\072\224",	/* AE ligature */
	"\124\244\225\227\250\310\304\70\251\272\311\270",	/* a ring */
	"\050\270\264\044\304\051\313",	/* i acute */
	"\044\225\227\250\270\307\305\264\244\024\310",	/* o slash */
	"\125\304\265\267\310\327\306\246\225\244\265\067\250\227",	/* ae ligature */
	"\024\231\252\312\331\324\026\326\054\253\114\313",	/* A diaresis */
	"\050\270\264\044\304\053\311",	/* i grave */
	"\044\225\231\252\312\331\325\304\244\054\253\114\313",	/* O diaresis */
	"\032\225\244\304\325\332\054\253\114\313",	/* U diaresis */
	"\124\224\232\332\027\307\052\315",	/* E acute */
	"\050\270\264\044\304\052\251\112\311",	/* i diaresis */
	"\044\251\272\311\310\267\306\304\264\245",	/* sharp s */
	"\044\225\231\252\312\331\325\304\244\033\275\333",	/* O circumflex */

	/* 0x60 ... 0x6f        */
	"\024\231\252\312\331\324\026\326\052\315",	/* A acute */
	"\024\231\252\312\331\324\026\326\033\254\312\333",	/* A tilde */
	"\124\244\225\227\250\310\304\033\254\312\333",	/* a tilde */
	"\024\232\312\331\325\304\224\027\267",	/* Eth */
	"\044\225\226\247\307\326\325\304\244\032\307\030\272",	/* eth */
	"\024\324\064\272\032\332\052\315",	/* I acute */
	"\024\324\064\272\032\332\055\312",	/* I grave */
	"\044\225\231\252\312\331\325\304\244\052\315",	/* O acute */
	"\044\225\231\252\312\331\325\304\244\055\312",	/* O grave */
	"\044\225\231\252\312\331\325\304\244\033\254\312\333",	/* O tilde */
	"\044\225\227\250\270\307\305\264\244\032\253\311\332",	/* o tilde */
	"\025\244\304\325\326\307\247\230\231\252\312\331\034\272\334",	/* czech S */
	"\110\250\227\246\266\305\264\224\052\270\312",	/* czech s */
	"\032\225\244\304\325\332\052\315",	/* U acute */
	"\032\231\266\264\066\331\332\054\253\114\313",	/* Y diaresis */
	"\022\326\330\030\226\264\052\251\112\311",	/* y diaresis */



	/* 0x70 ... 0x7f        */
	"\024\264\044\252\032\272\046\306\327\310\250",	/* Thorn */
	"\024\264\044\252\032\272\045\306\310\271\251",	/* thorn */
	"\067\267",		/*centered dot */
	"\024\250\246\265\306\310\106\325",	/* mu */
	"\064\272\104\312\252\231\250\270",	/* section */
	"\052\272\271\270\250\051\271\047\307\066\245\305\066\264",	/* three quarters */
	"\047\307",		/* dash */
	"\051\272\270\047\307\066\245\305\066\264",	/* one quarter */
	"\051\272\270\047\307\046\266\265\244\264",	/* one half */
	"\052\312\310\250\251\311\047\307",	/* _a */
	"\051\272\311\270\251\047\307",	/* _o */
	"\071\227\265\131\267\325",	/* much less than */
	"\045\250\330\325\245",	/* square */
	"\031\267\225\071\327\265",	/* much greater than */
	"\065\271\027\327\025\325",	/* plusminus */
	""
};
