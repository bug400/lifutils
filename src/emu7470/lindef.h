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


#ifndef __LINDEF_H
#define __LINDEF_H

typedef enum { LT_solid, LT_adaptive, LT_plot_at, LT_fixed } LineType;   

extern double     CurrentLinePatLen;
extern LineType   CurrentLineType;
extern signed int CurrentLinePattern;

#define LT_MIN -8
#define LT_ZERO 0
#define LT_MAX  8

#define LT_PATTERNS  ((LT_MIN*-1) + 1 + LT_MAX)         /* -8 .. 0 ..  +8  */
#define LT_ELEMENTS   20

#define LT_PATTERN_TOL 0.005  /* 0.5% of pattern length */

typedef signed char SCHAR;    

typedef double LINESTYLE[LT_PATTERNS][LT_ELEMENTS+1];

extern  LINESTYLE lt;

void set_line_style_defaults(void);
void set_line_style(SCHAR index, ...);
void set_line_style_by_UL(void);


#endif /* __LINDEF_H */
