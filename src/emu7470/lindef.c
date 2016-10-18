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

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include "emu7470.h"
#include "lindef.h"


double CurrentLinePatLen;
/*LineType   CurrentLineType;*/
signed int CurrentLinePattern;

LINESTYLE lt;


/********************************************
 * Line Style
 ********************************************/

void set_line_style(SCHAR index, ...)
{
	SCHAR count;
	double factor, percentage;
	va_list ap;

	SCHAR val;

	va_start(ap, index);

	for (count = 0, percentage = 0; count < LT_ELEMENTS; count++) {

		val = va_arg(ap, int);

		if (val < 0) {
			break;
		} else {
			lt[index - LT_MIN][count] = (double) val;
			percentage += val;
		}
	}

	lt[index - LT_MIN][count] = -1;

	if (fabs(percentage - 100.) > 0.5) {
		factor = 100.0 / percentage;
		for (count = 0; count < LT_ELEMENTS; count++) {
			if (lt[index - LT_MIN][count] < 0) {
				break;
			} else {
				lt[index - LT_MIN][count] *= factor;
			}
		}
	}

	va_end(ap);

}

void set_line_style_by_UL()
{
	SCHAR index, pos_index, neg_index, count, i;
	double factor, percentage;
	float tmp;

	if (read_float(&tmp)) {
		set_line_style_defaults();	/* reset to defaults */
		return;
	} else {
		index = (int) tmp;
	}

	pos_index = index - LT_MIN;
	neg_index = (index * -1) - LT_MIN;

	for (count = 0, percentage = 0; (read_float(&tmp) == 0); count++) {	/* while there is an argument */
		lt[pos_index][count] = (double) tmp;
		percentage += (int) tmp;
	}

	lt[pos_index][count] = -1;

	if (fabs(percentage - 100.) > 0.5) {
		factor = 100.0 / percentage;
		for (count = 0; count < LT_ELEMENTS; count++) {
			if (lt[pos_index][count] < 0) {
				break;
			} else {
				lt[pos_index][count] *= factor;
			}
		}
	}
	/* now derive the adaptive version */

	count--;

	if (count % 2) {	/* last value denotes a gap */
		lt[neg_index][0] = lt[pos_index][0] / 2;
		for (i = 1; i <= count; i++)
			lt[neg_index][i] = lt[pos_index][i];
		lt[neg_index][count + 1] = lt[pos_index][0] / 2;
		lt[neg_index][count + 2] = -1;
	} else {		/* last value denotes a line */
		lt[neg_index][0] =
		    (lt[pos_index][0] + lt[pos_index][count]) / 2;
		for (i = 1; i < count; i++)
			lt[neg_index][i] = lt[pos_index][i];
		lt[neg_index][count] = lt[neg_index][0];
		lt[neg_index][count + 1] = -1;
	}

}


void set_line_style_defaults()
{
/*                 Line  gap   Line  gap   Line   gap   Line  TERM        */

	set_line_style(-8, 25, 10, 0, 10, 10, 10, 0, 10, 25, -1);
	set_line_style(-7, 35, 10, 0, 10, 0, 10, 35, -1);
	set_line_style(-6, 25, 10, 10, 10, 10, 10, 25, -1);
	set_line_style(-5, 35, 10, 10, 10, 35, -1);
	set_line_style(-4, 40, 10, 0, 10, 40, -1);
	set_line_style(-3, 35, 30, 35, -1);
	set_line_style(-2, 25, 50, 25, -1);
	set_line_style(-1, 0, 100, 0, -1);
	set_line_style(0, 0, 100, -1);
	set_line_style(1, 0, 100, -1);
	set_line_style(2, 50, 50, -1);
	set_line_style(3, 70, 30, -1);
	set_line_style(4, 80, 10, 0, 10, -1);
	set_line_style(5, 70, 10, 10, 10, -1);
	set_line_style(6, 50, 10, 10, 10, 10, 10, -1);
	set_line_style(7, 70, 10, 0, 10, 0, 10, -1);
	set_line_style(8, 50, 10, 0, 10, 10, 10, 0, 10, -1);
}
