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

/** CHARDRAW.c: Implementation of characters by draw/move commands
 **
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "emu7470.h"
#include "lindef.h"
#include "chardraw.h"
#include "charset0.h"
#include "charset1.h"
#include "charset2.h"
#include "charset3.h"
#include "charset4.h"
#include "charset7.h"

extern HPGL_Pt Pen_pos, P1, P2;

extern int iwflag;
extern short scale_flag;
extern HPGL_Pt C1, C2;
extern HPGL_Pt S1, Q;
TEXTPAR TEXTP, *tp = &TEXTP;



static void code_to_ucoord(char c, HPGL_Pt * pp)
/**
 ** Converts internal one-byte code (in c) for a character vector
 ** into HP-GL coordinates (pointed to by pp)
 **/
{
	double x, y;

	/*
	 * RS6000 bug fix:
	 *    outer braces of casts removed, costing 2 double ops
	 * My guess: "char" is unsigned on RS6000
	 */
	x = (double) (c >> 4) - 1.0;	/* Bits 4,5,6 --> value 0..7 */
	y = (double) (c & 0x0f) - 4.0;	/* Bits 0-3   --> value 0..f */

	pp->x = tp->Txx * x + tp->Txy * y + tp->refpoint.x + tp->offset.x;
	pp->y = tp->Tyx * x + tp->Tyy * y + tp->refpoint.y + tp->offset.y;
}




static void ASCII_to_char(int c)
/**
 ** Main user interface: Convert ASCII code c into a sequence
 ** of move/draw vectors which draw a corresponding character
 **/
{
	HPGL_Pt p;
	char *ptr;
	int SafeLineType = CurrentLineType;

	CurrentLineType = LT_solid;

	switch (tp->font) {
	case 0:		/* charset 0, limited to 7 bit ASCII - 8bit addressing maps to charset 7        */

		if (c & 0x80) {
			c += 128;
			ptr = &charset7[c][0];
		} else {
			ptr = &charset0[c][0];
		}
		break;

	case 1:		/* charset 1, 9825      */

		if (c & 0x80) {
			set_Error(3,"Illegal character");
			c = ' ';
		}
		if (c == 95 || c == 96 || c == 126) {	/* backspacing for special characters  */
			tp->refpoint.x -= tp->chardiff.x;
			tp->refpoint.y -= tp->chardiff.y;
		}
		ptr = &charset1[c][0];
		break;

	case 2:		/* charset 2, French/German     */

		if (c & 0x80) {
			set_Error(3,"Illegal character");
			c = ' ';
		}
		if (c == 39 || c == 94 || c == 95 || c == 96 || c == 123 || c == 124 || c == 125) {	/* backspacing for special characters  */
			tp->refpoint.x -= tp->chardiff.x;
			tp->refpoint.y -= tp->chardiff.y;
		}
		ptr = &charset2[c][0];
		break;

	case 3:		/* charset 3, Scandinavian      */

		if (c & 0x80) {
			set_Error(3,"Illegal character");
			c = ' ';
		}
		if (c == 95 || c >= 123) {	/* backspacing for special characters  */
			tp->refpoint.x -= tp->chardiff.x;
			tp->refpoint.y -= tp->chardiff.y;
		}
		ptr = &charset3[c][0];
		break;

	case 4:		/* charset 4, Spanish/Latin American    */

		if (c & 0x80) {
			set_Error(3,"Illegal character");
			c = ' ';
		}
		if (c == 39 || c == 94 || c == 95 || c >= 123) {	/* backspacing for special characters  */
			tp->refpoint.x -= tp->chardiff.x;
			tp->refpoint.y -= tp->chardiff.y;
		}
		ptr = &charset4[c][0];
		break;

	default:
		if (c & 0x80) {
			c += 128;
			ptr = &charset7[c][0];
		} else {
			ptr = &charset0[c][0];
		}
		break;
	}

	for (; *ptr; ptr++) {	/* Draw this char */
		code_to_ucoord(*ptr & 0x7f, &p);

		if ((*ptr & 0x80) )	/* High bit is draw flag */
			Pen_action_to_tmpfile0(DRAW_TO, &p, FALSE);
		else
			Pen_action_to_tmpfile0(MOVE_TO, &p, FALSE);

	}

	/* Update cursor: to next character origin!   */

	tp->refpoint.x += tp->chardiff.x;
	tp->refpoint.y += tp->chardiff.y;
	CurrentLineType = SafeLineType;

}


/**********************************************************************/
void init_text_par(void)
{
	tp->width = 0.0075 * (P2.x - P1.x);
	tp->height = 0.015 * (P2.y - P1.y);
	tp->espace = 0.0;
	tp->eline = 0.0;
	tp->dir = 0.0;
	tp->slant = 0.0;
	tp->font = 0;
	tp->orig = 1;		/* Font number: 0 = old */
	tp->refpoint = tp->CR_point = Pen_pos;
	tp->offset.x = tp->offset.y = 0.0;
	tp->CR_offset.x= tp->CR_offset.y= 0.0;
	tp->ref_offset.x= tp->ref_offset.y=0.0;
	adjust_text_par();
}




void adjust_text_par(void)
/**
 ** Width, height, space, line, dir, slant
 ** as given in structure declaration
 **/
{
	double cdir, sdir;

/**
 ** Here, we use space & line as basic data, since these parameters
 ** are affected by SI and SR commands, not width or height!
 **/
	tp->space = tp->width * 1.5;
	tp->line = tp->height * 2.0;

	cdir = cos(tp->dir);
	sdir = sin(tp->dir);
	tp->Txx = tp->width * cdir / 4.0;
	tp->Tyx = tp->width * sdir / 4.0;
	tp->Txy = tp->height * (tp->slant * cdir - sdir) / 6.0;
	tp->Tyy = tp->height * (tp->slant * sdir + cdir) / 6.0;

	tp->chardiff.x = tp->space * (1.0 + tp->espace) * cdir;
	tp->chardiff.y = tp->space * (1.0 + tp->espace) * sdir;
	tp->linediff.x = tp->line * (1.0 + tp->eline) * sdir;
	tp->linediff.y = -tp->line * (1.0 + tp->eline) * cdir;
	Eprintf("Adjust text par chardiff x %f y %f\n",tp->chardiff.x,tp->chardiff.y);
	Eprintf("Adjust text par linediff x %f y %f\n",tp->linediff.x,tp->linediff.y);

}

void plot_string(char *txt, LB_Mode mode, short current_pen)
/**
 ** String txt cannot simply be processed char-by-char. Depending on
 ** the current label mode, its origin must first be calculated properly.
 ** Then, there are some special control characters which affect cursor
 ** position but don't draw anything. Finally, characters can be drawn
 ** one-by-one.
 **/
{

/*
        we got an offset from the CP command
*/
	if ((tp->ref_offset.x != 0.0) || (tp->ref_offset.y !=0.0)) {
		tp->refpoint.x = tp->CR_point.x+tp->ref_offset.x;
		tp->refpoint.y = tp->CR_point.y+tp->ref_offset.y;
		tp->ref_offset.x= 0.0;
		tp->ref_offset.y= 0.0;
	} else {
		tp->refpoint= Pen_pos;
	}
	Eprintf("In plot string refpoint %f %f\n",tp->refpoint.x,tp->refpoint.y);
	Eprintf("In plot string CR_point %f %f\n",tp->CR_point.x,tp->CR_point.y);
	while (*txt) {
		switch (*txt) {
		case ' ':
			tp->refpoint.x += tp->chardiff.x;
			tp->refpoint.y += tp->chardiff.y;
			break;
		case _CR:
			Eprintf("In plot string before _CR refpoint %f %f\n",tp->refpoint.x,tp->refpoint.y);
			Eprintf("In plot string after _CR mode vert refpoint %f %f\n",tp->refpoint.x,tp->refpoint.y);
			tp->refpoint = tp->CR_point; 
			Eprintf("In plot string after _CR mode refpoint= CR_point  %f %f\n",tp->refpoint.x,tp->refpoint.y);
			break;
		case _LF:
			Eprintf("In plot string before _LF refpoint %f %f\n",tp->refpoint.x,tp->refpoint.y);
			tp->CR_point.x += tp->linediff.x;
			tp->CR_point.y += tp->linediff.y;
			tp->refpoint.x += tp->linediff.x;
			tp->refpoint.y += tp->linediff.y;
			Eprintf("In plot string after _LF refpoint %f %f\n",tp->refpoint.x,tp->refpoint.y);
			break;
		case _BS:
			tp->refpoint.x -= tp->chardiff.x;
			tp->refpoint.y -= tp->chardiff.y;
			break;
		case _HT:
			tp->refpoint.x -= 0.5 * tp->chardiff.x;
			tp->refpoint.y -= 0.5 * tp->chardiff.y;
			break;
		case _VT:
			tp->CR_point.x -= tp->linediff.x;
			tp->CR_point.y -= tp->linediff.y;
			tp->refpoint.x -= tp->linediff.x;
			tp->refpoint.y -= tp->linediff.y;
			break;
		case _SO:
			if (tp->altfont)
				tp->font = tp->altfont;
			break;
		case _SI:
			tp->font = tp->stdfont;
			break;
		default:
				ASCII_to_char((int) *txt);
			break;
		}
/**
 ** Move to next reference point, e. g. the next character origin
 **/
		Pen_action_to_tmpfile0(MOVE_TO, &tp->refpoint, FALSE);
		txt++;
	}
	Pen_pos= tp->refpoint;
	Eprintf("Pen pos updated (LB) %f %f\n",Pen_pos.x, Pen_pos.y);
	Update_commanded_position(&Pen_pos,scale_flag);
}





static void ASCII_set_center(int c)
/**
 ** Convert ASCII code c into a sequence of move/draw vectors
 ** and determine their "center of gravity"
 **/
{
	HPGL_Pt p, center;
	int cnt;
	char *ptr;


	if (c & 0x80) {
		c += 128;
		ptr = &charset7[c][0];
	} else {
		ptr = &charset0[c][0];
	}

	center.x = center.y = 0.0;
	for (cnt = 0; *ptr; ptr++, cnt++) {	/* Scan this char */
		code_to_ucoord(*ptr & 0x7f, &p);
		center.x += p.x;
		center.y += p.y;
	}
	if (cnt) {
		tp->offset.x = -center.x / cnt;
		tp->offset.y = -center.y / cnt;
	} else			/* Should never happen:     */
		tp->offset.x = tp->offset.y = 0.0;
}




static void set_symbol_center(char c)
/**
 ** Symbol plotting requires a special x and y offset for proper
 ** symbol-specific centering
 **/
{
	tp->refpoint.x = 0.0;
	tp->refpoint.y = 0.0;
	tp->offset.x = 0.0;
	tp->offset.y = 0.0;
	ASCII_set_center(c);
	tp->refpoint.x = Pen_pos.x;	/*  - tp->chardiff.x / 2.0; */
	tp->refpoint.y = Pen_pos.y;	/*  - tp->chardiff.y / 2.0; */
}




void plot_symbol_char(char c)
/**
 ** Special case: Symbol plotting. This requires a special
 ** x and y offset (for proper centering) but then simply amounts to
 ** drawing a single character.
 **/
{
	set_symbol_center(c);

		ASCII_to_char((int) c);

/**
 ** Move to next reference point, e. g. the next character origin
 **/
	Pen_action_to_tmpfile0(MOVE_TO, &tp->refpoint, FALSE);
}


