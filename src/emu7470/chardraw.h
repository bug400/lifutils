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

/** chardraw.h
 **/


#define JOFF    4

#define	_BS	'\010'		/* Backspace            */
#define	_HT	'\011'		/* Horizontal Tab       */
#define	_LF	'\012'		/* Line Feed            */
#define	_VT	'\013'		/* Vertical Tab         */
#define	_CR	'\015'		/* Carriage Return      */
#define	_SO	'\016'		/* Shift Out            */
#define	_SI	'\017'		/* Shift In             */

/**
 ** Description of struct TextPar (used for internal font drawing):
 **
 ** A character size is defined by its WIDTH and HEIGHT.
 ** The distance from char. to char. is given by SPACE, from
 ** text line to text line by LINE.
 ** WIDTH, SPACE are fractions of the P1,P2 window width,
 ** SPACE, LINE  are fractions of the P1,P2 window height.
 ** The writing direction is DIR, which is the angle [0,2*M_PI]
 ** between text line & x direction.
 ** Set SLANT to an angle != 0 if characters are to appear e.g. italics-like.
 ** Stroked fonts selectable by setting FONT to > 0
 **      (not yet supported).
 **
 ** NOTE: struct TextPar was inherited from former project "plib" (a plot
 **       library which also featured character drawing). It was not
 **       designed from scratch for the purpose of HP-GL font management.
 **       However, for charset 0 it does a fair job here, mainly because
 **       plib itself had been inspired by HP-GL.
 **/

typedef struct {
	float width;		/* Width of a char (x dirc.)            */
	float height;		/* Height of a char (y dirc.)           */
	float space;		/* Distance between characters          */
	float line;		/* Distance betw. char. lines           */
	float espace;		/* Extra char space rel. to 'space'     */
	float eline;		/* Extra line space rel. to 'line'      */
	float dir;		/* Direction to x axis (rad)            */
	float slant;		/* Character slant (tan angle)          */
	int font;		/* Active Font number                   */
	int stdfont;		/* Designated tandard font number       */
	int altfont;		/* Designated alternate font number     */
	int orig;		/* Label origin code                    */


/**
 ** Internally needed for character resizing and positioning
 **
 ** T = matrix, mapping (relative) sprite coordinates into norm coord.,
 ** chardiff & linediff are used to advance the graphical text cursor,
 ** pref is a pointer to the current text reference point (origin):
 **/

	double Txx, Txy, Tyx, Tyy;	/* Transformation matrix        */
	HPGL_Pt chardiff,	/* Horiz. distance between characters   */
	 linediff,		/* Vertical distance between characters */
	 refpoint,		/* Current reference point      */
	 CR_point,		/* Returns point after a <CR>   */
	 offset,		/* Needed for HP-GL command ``LO;''     */
         ref_offset,            /* offset for Ref point after CP command */
         CR_offset;             /* offset for CR_Point after CP command */
	double strokewidth;	/* current stroke weight (or 9999. for current PW */
	double sstrokewidth;	/* stdfont stroke weight (or 9999. for current PW */
	double astrokewidth;	/* altfont stroke weight (or 9999. for current PW */
} TEXTPAR, *TextPar;




typedef enum { LB_direct, LB_buffered, LB_buffered_in_use } LB_Mode;	/* LB and PB work differently ! */

/**
 ** Prototypes:
 **/

#ifdef	__cplusplus
extern "C" {
#endif

/* void	code_to_ucoord	(char, HPGL_Pt *); */

	int init_font(int);
	void init_text_par(void);
	void adjust_text_par(void);
/* void	ASCII_to_char	(int);*/
	void plot_string(char *, LB_Mode, short);
	void plot_symbol_char(char);

#ifdef	__cplusplus
}
#endif
