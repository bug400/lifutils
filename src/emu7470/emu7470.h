#ifndef	__EMU7470_H
#define	__EMU7470_H
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

/** emu7470.h: Header for Project "emu7470"
 **/


#ifndef M_PI
#define M_PI	3.141592654	/* Some machines don't know PI */
#endif

#ifndef	MIN
#define MIN(x,y)  ((x)<(y) ? (x) : (y))
#endif

#ifndef	MAX
#define MAX(x,y)  ((x)<(y) ? (y) : (x))
#endif


#define	TRUE	1
#define	FALSE	0

#define MIN_INT -32768		/* parameter checking */
#define MAX_INT 32767

#define	HYPOT(x,y)	hypot(x,y)
#define MAXCMDBUF 1024



/**
 ** Exit codes
 **/


#define ERROR (-1)
#define NOERROR 0
#define COPYNOTE 1


#define	ESC	'\033'

#define	MAX_LB_LEN	150	/* Max num of chars per label   */

#define MAXPOLY 20480		/* Size of polygon vertex buffer */

/**
 ** Misc. typedefs
 **/

typedef unsigned char Byte;



/**
 ** When adding your special mode, add a symbol here.
 ** Please note the alphabetical order (and keep it).
 **/



typedef struct {
	float x, y;
} HPGL_Pt;


typedef enum {
	CLEAR, MOVE_TO, DRAW_TO, PLOT_AT, SET_PEN, CMD_OUTPUT, CMD_STATUS, CMD_EMSG, CMD_DIGI_START, CMD_DIGI_CLEAR, CMD_P1P2, CMD_EOF
} PlotCmd;

typedef enum {
   A4, US
} PaperSize;



/**
 ** Prototypes:
 **/


void Eprintf(const char *, ...);

void read_HPGL(void);
void Pen_action_to_tmpfile(PlotCmd, const HPGL_Pt *, int);
void Pen_action_to_tmpfile0(PlotCmd, const HPGL_Pt *, int);
void Update_commanded_position(const HPGL_Pt *, int);

int read_float(float *);
double ceil_with_tolerance(double, double);
void line(int relative, HPGL_Pt p);

void fill(HPGL_Pt polygon[], int numpoints, HPGL_Pt P1, HPGL_Pt P2,
	  int scale_flag, int filltype, float spacing, float hatchangle);
void set_Error(short errnumber, const char * msg);

void read_string (char *target);
void PlotCmd_to_tmpfile (PlotCmd cmd);

void HPGL_Pt_to_tmpfile (const HPGL_Pt * pf);

void HPGL_Pt_to_polygon (const HPGL_Pt pf );

void clear_Error(void);

int clipline(double *x1, double *y1, double *x2, double *y2);

int ComputeOutCode(double x, double y);

void check_cmdbuf(void);

/*std_main*/

#endif				/*      __EMU7470_H       */
