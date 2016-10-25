/* 
   emu7470 emulator engine for a HP7470A plotter for use
   with the pyILPER software

   Derived from HP2XX
   Copyright (c) 1991 - 1994 Heinz W. Werntges
   Parts Copyright (c) 1999 - 2001 Martin Kroeker
   emu7470 (C) 2016 Joachim Siebold
   
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

   This is the HP7470A emulation engine for pyILPER. This program is
   started as a separate process of pyILPER. The engine gets HP-IL commands
   on standard input and returns several kinds of information. The communication
   is text only.

   The communication in detail:

   pyILPER send a record with the plotter status byte and the complete HP-IL
   command to emu7470. The status byte and the HP-IL command are base16 encoded
   because HP-IL label commands may contain non printable characters.

   emu7470 returns always the following records:

   - STATUS: status record with status byte, error code and current HP-GL label term char
   - EOF: end of information record

   The plotter status byte is "shared" betwenn pyILPER and the plotter engine, 
   because it can be modified by the plotter engine (i.e. set data available bit)
   and pyILPER (clear data available bit) as well.

   Depending on processing of the HP-GL commands em7470 can return the
   following records to pyILPER:
   - CLEAR: issue pyILPER to clear the plotter output view(s)
   - MOVE_TO, DRAW_TO, PLOT_AT, SET_PEN: draw graphics on the pyILPER plotter output view(s)
   - EMSG: error message
   - OUTPUT: data to be sent to HP-IL (i.e. pen position)
   - DIGI_START, DIGI_CLEAR: issue pyILPER to enter and to leave digitizing mode
   - P!P2: send corrdinates of the scaling point to pyILPER

   Changelog
   23.10.2016 jsi
   - output version number, supporess output of P1P2 when first initialized
   - update commanded position, if position overflow in PD;PU;PA;PR
   25.10.2016 jsi
   - added debug output (lost mode), corrected debug output
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */


#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include "emu7470.h"
#include "chardraw.h"
#include "lindef.h"

#define	ETX		'\003'

#define P1X_default	250.0	/* drop margins       */
#define P1Y_default	279.0
#define P2X_default   10250.0	/* HP7470 default */
#define P2Y_default   7479.0



/**
 ** Globals needed in other source files:
 **/
LineType CurrentLineType = LT_solid;
short scale_flag = FALSE;
short error=0;
short status=0x08; /* status: initialized */
short mask=223;

char * cmdbuf= (char *) NULL;
int cmdbuf_ptr;

HPGL_Pt Pen_pos = { 0, 0 };			/* Actual commanded plotter pen pos */
HPGL_Pt Cmd_pos = { 0, 0 };                     /* Actual commanded position */
HPGL_Pt Pen_pos_clipped= { -1.0, -1.0 };        /* Pen position if line was clipped */
HPGL_Pt P1 = { P1X_default, P1Y_default };	/* Scaling points */
HPGL_Pt P2 = { P2X_default, P2Y_default };
HPGL_Pt S1 = { P1X_default, P1Y_default };	/* Scaled       */
HPGL_Pt S2 = { P2X_default, P2Y_default };	/* points       */
HPGL_Pt Q = { 1., 1. };		/* Delta-P/Delta-S: Initialized with first SC   */
HPGL_Pt M;			/* maximum coordinates set by PS instruction */
HPGL_Pt digi = { -1.0,-1.0};
/**
 ** Global from chardraw.c:
 **/
extern TextPar tp;

/**
 ** "Local" globals (I know this is messy...) :
 **/
static float neg_ticklen, pos_ticklen;
static double Diag_P1_P2, pat_pos;
static HPGL_Pt p_last = { M_PI, M_PI };	/* Init. to "impossible" values */
static HPGL_Pt hwlimit= { 10900.0,  7650.0};
static HPGL_Pt ptemp= {0.0, 0.0};

static int maxpens=2;

static short mv_flag = FALSE;
static short ct_dist = FALSE;
static short pen = -1;
static short pen_down = FALSE;	/* Internal HP-GL book-keeping: */
static short plot_rel = FALSE;
static short lost_mode= FALSE;
static char StrTerm = ETX;	/* String terminator char       */
static short StrTermSilent = 1;	/* only terminates, or prints too */
static char *strbuf = NULL;
static unsigned int strbufsize = MAX_LB_LEN + 1;
static unsigned int cmdbufsize = MAXCMDBUF+1;
static char symbol_char = '\0';	/* Char in Symbol Mode (0=off)  */

static int out_cmd;		/* generated command */
static int out_last_x;          /* remember last point */
static int out_last_y;
static int XMin;                /* clip rectangle */
static int XMax;
static int YMin;
static int YMax;
static PaperSize psize= A4;
static int first_init= TRUE;   /* initial init */

static const signed char
hex_decode_table[0x80] =
  {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -2, -2, -1, -1, -2, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

/* Known HPGL commands, ASCII-coded as High-byte/low-byte int's */

#define AA	0x4141
#define AR	0x4152
#define AP      0x4150		/* NOP */
#define AF      0x4146          /* NOP */
#define AH      0x4148		/* NOP */
#define CA      0x4341		/*MK */
#define CI	0x4349
#define CP	0x4350
#define CS      0x4353		/*MK */
#define DC      0x4443
#define DF	0x4446
#define DI	0x4449
#define DP      0x4450
#define DR	0x4452
#define DT	0x4454
#define EC	0x4543		/* NOP */
#define IN	0x494E
#define IM	0x494D
#define IP	0x4950
#define IW      0x4957		/*MK */
#define LB	0x4C42
#define LT	0x4C54
#define OA      0x4F41
#define OC      0x4F43
#define OD      0x4F44
#define OE      0x4F45
#define OF      0x4F46
#define OI      0x4F49
#define OO      0x4F4F
#define OP	0x4F50
#define OS      0x4F53
#define OW	0x4F57
#define PA	0x5041
#define PD	0x5044
#define PR	0x5052
#define PU	0x5055
#define SA      0x5341		/*MK */
#define SC	0x5343
#define SI	0x5349
#define SL	0x534C
#define SM	0x534D
#define SP	0x5350
#define SR	0x5352
#define SS      0x5353		/*MK */
#define TL	0x544C
#define UC	0x5543
#define VA	0x5641		/* NOP */
#define VN	0x564E		/* NOP */
#define VS	0x5653
#define XT	0x5854
#define YT	0x5954
#define ZY      0x5A59
#define ZZ      0x5A5A

static int check_value(float value, float min, float max)
{
	if (value < min || value > max ) {
		set_Error(3,"Value out of range");
		return TRUE;
	} else
		return FALSE;
}


static void reset_HPGL(void)
{
/*
	needs checking
*/
	p_last.x = p_last.y = M_PI;
	out_last_x=0;
	out_last_y=0;
        Pen_pos_clipped.x= Pen_pos_clipped.y=-1.0;
	pen_down = FALSE;
	plot_rel = FALSE;
	pen = -1;
	mv_flag = FALSE;
	ct_dist = FALSE;
	CurrentLineType = LT_solid;
	lost_mode= FALSE;

	set_line_style_defaults();
	StrTerm = ETX;
	StrTermSilent = 1;
	if (strbuf == NULL) {
		strbuf = malloc(strbufsize);
		if (strbuf == NULL) {
			Eprintf("%s\n","No memory!");
			exit(ERROR);
		}
	}
	strbuf[0] = '\0';
	if (cmdbuf == (char *) NULL) {
		cmdbuf= malloc(cmdbufsize);
		if (cmdbuf == NULL) {
			Eprintf("%s\n","No memory!");
			exit(ERROR);
		}
	}


	Diag_P1_P2 = /*@-unrecog@ */ HYPOT(P2.x - P1.x, P2.y - P1.y);
	CurrentLinePatLen = 0.04 * Diag_P1_P2;
	pat_pos = 0.0;
	scale_flag = FALSE;
	S1 = P1;
	S2 = P2;
	Q.x = Q.y = 1.0;
	Pen_pos.x = Pen_pos.y = 0.0;
	neg_ticklen = 0.005;	/* 0.5 %        */
	pos_ticklen = 0.005;
	symbol_char = '\0';
	init_text_par();
	error=0;
	mask=223;
	XMin=0;
	YMin=0;
	XMax=(int) floor(hwlimit.x);
        YMax=(int) floor(hwlimit.y);
        
}

static void init_HPGL(void)
{
/**
 ** Init HPGL-Parameters
 **/
	P1.x = P1X_default;
	P1.y = P1Y_default;
       	P2.x = P2X_default;
        P2.y = P2Y_default;
	if (psize== A4) {
		hwlimit.x= 10900.0;
		hwlimit.y= 7650.0;
	}
	else {
		hwlimit.x= 10300.0;
		hwlimit.y= 7650.0;
	}

	maxpens = 2;
	status=0x08; /* status: initialized */
	reset_HPGL();
        if (! first_init)
		printf("%d %d %d %d %d\n", CMD_P1P2, (int)floor(P1.x+0.5), (int) floor(P1.y+0.5), (int)floor(P2.x+0.5), (int) floor(P2.y+0.5));
}



static void User_to_Plotter_coord(const HPGL_Pt * p_usr, HPGL_Pt * p_plot)
/**
 ** 	Utility: Transformation from (scaled) user coordinates
 **	to plotter coordinates
 **/
{
	p_plot->x = P1.x + (p_usr->x - S1.x) * Q.x;
	p_plot->y = P1.y + (p_usr->y - S1.y) * Q.y;
}



static void Plotter_to_User_coord(const HPGL_Pt * p_plot, HPGL_Pt * p_usr)
/**
 ** 	Utility: Transformation from plotter coordinates
 **	to (scaled) user coordinates
 **/
{
	p_usr->x = S1.x + (p_plot->x - P1.x) / Q.x;
	p_usr->y = S1.y + (p_plot->y - P1.y) / Q.y;
}

/*
 Low level pen move/draw routine
*/

void PlotCmd_to_tmpfile(PlotCmd cmd)
{
	out_cmd= cmd;
}

void HPGL_Pt_to_tmpfile(const HPGL_Pt * pf)
{

	int x,y;
	double dx1,dy1,dx2,dy2;
        int x1, y1, x2, y2;
	int retcode;
/*
        check if we are in lost mode
*/
	if (pf->x < MIN_INT) {
		lost_mode= TRUE;
	}
	if (pf->x > MAX_INT) {
		lost_mode= TRUE;
	}
	if (pf->y < MIN_INT) {
		lost_mode= TRUE;
	}
	if (pf->y > MAX_INT) {
		lost_mode= TRUE;
	}
	if (lost_mode) {
		Eprintf("Position overflow %f %f\n",pf->x,pf->y);
		set_Error(6,"Position overflow");
		return;
	}

	x= (int) floor(pf->x+0.5);
	y= (int) floor(pf->y+0.5);

	/*
		PLOT AT: do nothing if we are outside the cip box
	*/
	if (out_cmd== PLOT_AT) {
		if (x>= XMin && y >= YMin && x<= XMax && y<= YMax) 
			printf("%d %d %d\n",PLOT_AT,x, y);
		out_last_x=x;
		out_last_y=y;
		return;
	}
	/*
		do clipping
	*/
	dx1= (double) out_last_x;
	dy1= (double) out_last_y;
	dx2= (double) pf->x;
	dy2= (double) pf->y;
	retcode=clipline(&dx1,&dy1,&dx2,&dy2);
	x1= (int) floor(dx1+0.5);
	y1= (int) floor(dy1+0.5);
	x2= (int) floor(dx2+0.5);
	y2= (int) floor(dy2+0.5);
	
	/* 
	if the line is completely outside, do nothing
	*/
        if (! retcode ) {
		out_last_x=x;
		out_last_y=y;
		return;
	}
	/*
		move to the point where we enter the clip box from outside
	*/
	if (fabs(dx1- out_last_x) > 0.005 || fabs(dy1- out_last_y) >0.005){
		printf("%d %d %d C\n",MOVE_TO,x1,y1);
	}
	/*
		move/draw to the point where we leave the clip box
	*/
	if (fabs(dx2-x) > 0.005 || fabs (dy2-y) > 0.005) {
		printf("%d %d %d C\n",out_cmd,x2,y2);
		Pen_pos_clipped.x= dx2;
		Pen_pos_clipped.y= dy2;
	} else {
		printf("%d %d %d C\n",out_cmd,x,y);
		Pen_pos_clipped.x= -1.0;
		Pen_pos_clipped.y= -1.0;
	}
	out_last_x=x;
	out_last_y=y;
}

/**
 **	Low-level vector generation & file I/O
 **/

static void
LPattern_Generator(HPGL_Pt * pa,
		   double dx, double dy,
		   double start_of_pat, double end_of_pat)
/**
 **	Generator of Line type patterns:
 **
 **	pa:		Start point (ptr) of current line segment
 **	dx, dy:		Components of c * (*pb - *pa), c holding
 **				dx^2 + dy^2 = pattern_length^2
 **	start_of_pat:	Fraction of start point within pattern
 **	end_of_pat:	Fraction of end   point within pattern
 **			Valid: 0 <= start_of_pat <= end_of_pat <= 1
 **
 **	A pattern consists of alternating "line"/"point" and "gap" elements,
 **	always starting with a line/point. A point is a line of zero length.
 **	The table below contains the relative lengths of the elements
 **	of all line types except LT0; and LT; (7), which are treated separately.
 **	These lengths always add up to 1. A negative value terminates a pattern.
 **/
{
	double length_of_ele, start_of_action, end_of_action;
	static double *p_cur_pat;

	p_cur_pat = lt[(LT_MIN * -1) + (int) CurrentLinePattern];	/* was CurrentLineType */

	if (CurrentLineType == LT_adaptive)
		for (;;) {
			length_of_ele = (double) *p_cur_pat++ / 100;	/* Line or point        */
			if (length_of_ele < 0.)
				return;
			if (length_of_ele < 1.e-5)
				PlotCmd_to_tmpfile(PLOT_AT);
			else
				PlotCmd_to_tmpfile(DRAW_TO);

			pa->x += dx * length_of_ele;
			pa->y += dy * length_of_ele;
			HPGL_Pt_to_tmpfile(pa);

			length_of_ele = (double) *p_cur_pat++ / 100;	/* Gap        */
			if (length_of_ele < 0.)
				return;
			pa->x += dx * length_of_ele;
			pa->y += dy * length_of_ele;
			PlotCmd_to_tmpfile(MOVE_TO);
			HPGL_Pt_to_tmpfile(pa);
	} else			/* LT_fixed */
		for (end_of_action = 0.0;;) {
	    /**
	     ** Line or point:
	     **/
			start_of_action = end_of_action;
			length_of_ele = (double) *p_cur_pat++ / 100;
			if (length_of_ele < 0.)
				return;

			if (length_of_ele < 1.e-5) {	/* Dot Only */
				PlotCmd_to_tmpfile(PLOT_AT);
				HPGL_Pt_to_tmpfile(pa);
			} else {	/* Line Segment */
				end_of_action += length_of_ele;

				if (end_of_action > start_of_pat) {	/* If anything to do:   */
					if (start_of_pat <= start_of_action) {	/* If start is valid    */
						if (end_of_action <= end_of_pat) {	/* Draw full element    */
							pa->x +=
							    dx *
							    length_of_ele;
							pa->y +=
							    dy *
							    length_of_ele;
							PlotCmd_to_tmpfile
							    (DRAW_TO);
							HPGL_Pt_to_tmpfile
							    (pa);
						} else
							/* End_of_action beyond End_of_pattern:   */
						{	/* --> Draw only first part of element: */
							pa->x +=
							    dx *
							    (end_of_pat -
							     start_of_action);
							pa->y +=
							    dy *
							    (end_of_pat -
							     start_of_action);
							PlotCmd_to_tmpfile
							    (DRAW_TO);
							HPGL_Pt_to_tmpfile
							    (pa);
							return;
						}
					} else
						/* Start_of_action before Start_of_pattern:       */
					{
						if (end_of_action <= end_of_pat) {	/* Draw remainder of element            */
							pa->x +=
							    dx *
							    (end_of_action
							     -
							     start_of_pat);
							pa->y +=
							    dy *
							    (end_of_action
							     -
							     start_of_pat);
							PlotCmd_to_tmpfile
							    (DRAW_TO);
							HPGL_Pt_to_tmpfile
							    (pa);
						} else
							/* End_of_action beyond End_of_pattern:   */
							/* Draw central part of element & leave   */
						{
							if (end_of_pat ==
							    start_of_pat)
								PlotCmd_to_tmpfile
								    (PLOT_AT);
							else
								PlotCmd_to_tmpfile
								    (DRAW_TO);
							pa->x +=
							    dx *
							    (end_of_pat -
							     start_of_pat);
							pa->y +=
							    dy *
							    (end_of_pat -
							     start_of_pat);

							HPGL_Pt_to_tmpfile
							    (pa);
							return;
						}
					}
				}
			}
	    /**
	     ** Gap (analogous to line/point):
	     **/
			start_of_action = end_of_action;
			length_of_ele = (double) *p_cur_pat++ / 100;
			if (length_of_ele < 0)
				return;
			end_of_action += length_of_ele;
			if (end_of_action > start_of_pat) {	/* If anything to do:   */
				if (start_of_pat <= start_of_action) {	/* If start is valid    */
					if (end_of_action <= end_of_pat) {	/* Full gap             */
						pa->x +=
						    dx * length_of_ele;
						pa->y +=
						    dy * length_of_ele;
						PlotCmd_to_tmpfile
						    (MOVE_TO);
						HPGL_Pt_to_tmpfile(pa);
					} else
						/* End_of_action beyond End_of_pattern:   */
					{	/* --> Apply only first part of gap:    */
						pa->x +=
						    dx * (end_of_pat -
							  start_of_action);
						pa->y +=
						    dy * (end_of_pat -
							  start_of_action);
						PlotCmd_to_tmpfile
						    (MOVE_TO);
						HPGL_Pt_to_tmpfile(pa);
						return;
					}
				} else
					/* Start_of_action before Start_of_pattern:       */
				{
					if (end_of_action <= end_of_pat) {	/* Apply remainder of gap               */
						pa->x +=
						    dx * (end_of_action -
							  start_of_pat);
						pa->y +=
						    dy * (end_of_action -
							  start_of_pat);
						PlotCmd_to_tmpfile
						    (MOVE_TO);
						HPGL_Pt_to_tmpfile(pa);
					} else
						/* End_of_action beyond End_of_pattern:   */
						/* Apply central part of gap & leave      */
					{
						if (end_of_pat ==
						    start_of_pat)
							return;	/* A null move  */
						pa->x +=
						    dx * (end_of_pat -
							  start_of_pat);
						pa->y +=
						    dy * (end_of_pat -
							  start_of_pat);
						PlotCmd_to_tmpfile
						    (MOVE_TO);
						HPGL_Pt_to_tmpfile(pa);
						return;
					}
				}
			}
		}
}


double ceil_with_tolerance(double x, double tol)
{
	double rounded;

	rounded = (double) (x + 0.5);

	if (fabs(rounded - x) <= tol)
		return (rounded);
	else
		return (ceil(x));
}

static void Line_Generator(HPGL_Pt * pa, const HPGL_Pt * pb, int mv_flag)
{
	double seg_len, dx, dy, quot;
	int n_pat, i;

	dx = pb->x - pa->x;
	dy = pb->y - pa->y;
	seg_len = HYPOT(dx, dy);

	switch (CurrentLineType) {

	case LT_solid:
		if (seg_len < 1.e-8) {
			return;	/* No line to draw ??           */
		}
		PlotCmd_to_tmpfile(DRAW_TO);
		HPGL_Pt_to_tmpfile(pb);
		return;

	case LT_adaptive:
		if (seg_len < 1.e-8) {
			return;	/* No line to draw ??           */
		}
		pat_pos = 0.0;	/* Reset to start-of-pattern    */
		n_pat =
		    (int) ceil_with_tolerance(seg_len / CurrentLinePatLen,
					      CurrentLinePatLen *
					      LT_PATTERN_TOL);
		if (n_pat == 0) {	/* sanity check for segment << pattern length */
			n_pat = 1;
		}
		dx /= n_pat;
		dy /= n_pat;
		/* Now draw n_pat complete line patterns */
		for (i = 0; i < n_pat; i++)
			LPattern_Generator(pa, dx, dy, 0.0, 1.0);
		return;

	case LT_plot_at:
		PlotCmd_to_tmpfile(PLOT_AT);
		HPGL_Pt_to_tmpfile(pb);
		return;

	case LT_fixed:
		if (seg_len < 1.e-8) {
			return;	/* No line to draw ??           */
		}
		if (mv_flag)	/* Last move ends old line pattern      */
			pat_pos = 0.0;
		quot = seg_len / CurrentLinePatLen;
		dx /= quot;
		dy /= quot;
		while (quot >= 1.0) {
			LPattern_Generator(pa, dx, dy, pat_pos, 1.0);
			quot -= (1.0 - pat_pos);
			pat_pos = 0.0;
		}
		quot += pat_pos;
		if (quot >= 1.0) {
			LPattern_Generator(pa, dx, dy, pat_pos, 1.0);
			quot -= 1.0;
			pat_pos = 0.0;
		}
		if (quot > LT_PATTERN_TOL) {
			LPattern_Generator(pa, dx, dy, pat_pos, quot);
			pat_pos = quot;
		} else {
			PlotCmd_to_tmpfile(MOVE_TO);
			HPGL_Pt_to_tmpfile(pb);
		}
		return;

	default:
		break;
	}

}

void Pen_action_to_tmpfile(PlotCmd cmd, const HPGL_Pt * p, int scaled)
{
	HPGL_Pt P;


	if (scaled)		/* Rescaling    */
		User_to_Plotter_coord(p, &P);
	else
		P = *p;		/* Local copy   */


	Pen_pos = P;		/* Actual plotter pos. in plotter coord */
	Pen_action_to_tmpfile0(cmd, p, scaled);
	Update_commanded_position(&Pen_pos,scaled);
}

void Update_commanded_position(const HPGL_Pt *p, int scaled)
{
	Eprintf("commanded position %f %f %d \n",p->x, p->y,scaled);
	if (scaled)
		Plotter_to_User_coord(p,&Cmd_pos);
        else
		Cmd_pos= *p;
	if (Cmd_pos.y < MIN_INT)
		Cmd_pos.y=MIN_INT;
	if (Cmd_pos.x < MIN_INT)
		Cmd_pos.x=MIN_INT;
	if (Cmd_pos.x > MAX_INT)
		Cmd_pos.x=MAX_INT;
	if (Cmd_pos.y > MAX_INT)
		Cmd_pos.y=MAX_INT;

	Eprintf("commanded position %f %f %d \n",Cmd_pos.x, Cmd_pos.y,scaled);
}




void Pen_action_to_tmpfile0(PlotCmd cmd, const HPGL_Pt * p, int scaled)
{
	static HPGL_Pt P_last;
	HPGL_Pt P;


	if (scaled)		/* Rescaling    */
		User_to_Plotter_coord(p, &P);
	else
		P = *p;		/* Local copy   */

	/* Extreme values needed for later scaling:    */

	switch (cmd) {
	case MOVE_TO:
		mv_flag = TRUE;
		PlotCmd_to_tmpfile(MOVE_TO);
		HPGL_Pt_to_tmpfile(&P);
		break;

  /**
   ** Multiple-move suppression. In addition,
   ** a move only precedes a draw -- nothing else!
   **/

	case DRAW_TO:
		/* drop through */
	case PLOT_AT:
		Line_Generator(&P_last, &P, mv_flag);
		mv_flag = FALSE;
		break;

	default:
		exit(ERROR);
	}
	P_last = P;
}


int read_float(float *pnum)
/**
 ** Search for next number, skipping commas or blanks
 **	returns	0 if valid number
 **		1 if no number found
 **/
{
	int c, isneg=FALSE, numfound=FALSE;
	float divisor=10.0;

	*pnum=0.0;
	/* skip leading commas or blanks */
	while(TRUE) {
		c= cmdbuf[cmdbuf_ptr];
		if (c!= ' ' && c!= ',') break;
		cmdbuf_ptr++;
	}
	/* sign */
	if (c== '+') cmdbuf_ptr++;
	if (c== '-') {
		cmdbuf_ptr++;
		isneg= TRUE;
	}
	/* int part of number */
	while(isdigit(c= cmdbuf[cmdbuf_ptr])) {
		numfound=TRUE;
		*pnum= *pnum*10.0+ c - '0';
		cmdbuf_ptr++;
	}
	/* frac part of number */
	if (c== '.')  {
		cmdbuf_ptr++;
		while(isdigit(c= cmdbuf[cmdbuf_ptr])) {
			numfound=TRUE;
			*pnum= *pnum + (c-'0')/ divisor;
			divisor*= 10.0;
			cmdbuf_ptr++;
		}
	}
	if (isneg) *pnum= - *pnum;
	return(! numfound);
}

void read_string(char *buf)
{
	int c;
	unsigned int n;

	for (n = 0, c = cmdbuf[cmdbuf_ptr],cmdbuf_ptr++; (c != '\0') && (c != StrTerm);
	     c = cmdbuf[cmdbuf_ptr], cmdbuf_ptr++) {
		if (n > strbufsize / 2) {
			strbufsize *= 2;
			strbuf = realloc(strbuf, strbufsize);
			if (strbuf == NULL) {
				Eprintf("%s\n","No memory !");
				exit(ERROR);
			}
			buf = strbuf + n;
		}
		if (c == '\0')
			continue;	/* ignore \0 */
		if (n++ < strbufsize)
			*buf++ = c;
	}
	if (c == StrTerm && StrTermSilent == 0)
		*buf++ = c;
	*buf = '\0';
}


/****************************************************************************/

/*
 	lines:	Process PA-, PR-, PU-, and  PD- commands
*/
static void lines(int relative)
/*
 Examples of anticipated commands:

	PA PD0,0,80,50,90,20PU140,30PD150,80;
	PU0,0;PD20.53,40.32,30.08,60.2,40,90,;PU100,300;PD120,340...

	If parameters are included, both coordinates of an X,Y coordinate pair must be
	given. An odd number of parameters will set an error condition but all X,Y
	pairs which precede the unmatched parameter will be plotted.

	Recommandes parameters are decimal numbers between 32768.0 and 32767.9999.
	When scaling is off, parameters are truncated as follows:
	For positive numbers the factional part is truncated
	For negative numbers the factional part is discarded and the integer portion is
	changed to the next more negative number.
	When scaling is on any fractional part is used.
	
	The plotter discards parameters which are out of range. Error 3 will be set.
	If scaling is off in-range-parameters are >= -32768 and <= 32767.
	When scaling is on both the parameters and their plotter unit equivalent
	must be in the same range.

	Since out-of-range points are discarded, the plotter will draw a line between
	the two points on either side of the discarded points.

*/
{
	HPGL_Pt p;
	int numcmds = 0;

	for (;;) {
		if (read_float(&p.x)) {	/* No number found      */
			if (numcmds > 0)
				return;
			/*simulate dot created by 'real' pen on PD;PU; */
			if (pen_down && mv_flag && pen && (!lost_mode) > 0 ) {	
				/*but not on PDPA */
				p.x = p_last.x + 0.01;
				p.y = p_last.y + 0.01;
				Pen_action_to_tmpfile(MOVE_TO, &p,
						      scale_flag);
				Pen_action_to_tmpfile(DRAW_TO,
						      &p_last,
							      scale_flag);
			}
			return;
		}

		if (read_float(&p.y) ){ /* x without y invalid! */
			set_Error(2,"missing parameter");
                        return;
		}
		/*
			clamp values if scaling is off
		*/
		if (! scale_flag) {
			p.x=floor(p.x);
			p.y=floor(p.y);
		} 
		/*
			process pair of coordinates, check if values out of range
		*/

		line(relative, p);
		numcmds++;
	}
}


/*
 * line : process a pair of coordinates, check if values are out of range
 */
void line(int relative, HPGL_Pt p)
{
	HPGL_Pt porig,pp;
	/*
		check if the absolute coordinate values are out of range
	*/
	Eprintf("%s\n","in line");
	if (check_value(p.x,(float) MIN_INT, (float) MAX_INT) ||
		check_value(p.y,(float) MIN_INT, (float) MAX_INT)) {
                Update_commanded_position(&p,scale_flag);
		set_Error(3,"coordinate out of range");
		lost_mode= TRUE;
		return;
	}
	/*
		check if relative coordinate values out of range
	*/
	if (relative) {
		p.x += p_last.x;
		p.y += p_last.y;
	}
	if (check_value(p.x,(float) MIN_INT, (float) MAX_INT) ||
		check_value(p.y,(float) MIN_INT, (float) MAX_INT)) {
                Update_commanded_position(&p,scale_flag);
		set_Error(3,"resulting coordinate out of range");
		lost_mode= TRUE;
		return;
	}
	if (scale_flag) {
		User_to_Plotter_coord(&p, &pp);
		if (check_value(p.x,(float) MIN_INT, (float) MAX_INT) ||
			check_value(p.y,(float) MIN_INT, (float) MAX_INT)) {
			set_Error(3,"scaled coordinate out of range");
                	Update_commanded_position(&p,scale_flag);
			lost_mode= TRUE;
			return;
		}
	}
	/* if we are here in lost_mode we can reset that state */
	Eprintf("%s\n","reset lost mode");
	lost_mode= FALSE;

	porig.x = p.x;
	porig.y = p.y;


	if (pen_down ) {
		Pen_action_to_tmpfile(DRAW_TO, &p, scale_flag);
	} else {
		Pen_action_to_tmpfile(MOVE_TO, &p, scale_flag);
	}


	if (symbol_char) {
		plot_symbol_char(symbol_char);
		Pen_action_to_tmpfile(MOVE_TO, &p, scale_flag);
	}
	p_last = porig;

}


/**
 **	Arcs, circles and alike
 **/


static void arc_increment(HPGL_Pt * pcenter, double r, double phi)
{
	HPGL_Pt p;
	p.x = pcenter->x + r * cos(phi);
	p.y = pcenter->y + r * sin(phi);

	if (pen_down)
		Pen_action_to_tmpfile(DRAW_TO, &p, scale_flag);
	else if ((p.x != p_last.x) || (p.y != p_last.y))
		Pen_action_to_tmpfile(MOVE_TO, &p, scale_flag);
	p_last = p;
}
	/* The AA/AR instruction  requires that both X- and Y coordinates
	be specified (coordinate pair) in integer format. They are
	interpreted as plotter units if scaling is off or as user units if
	scaling is on. 

	The arc angle is in integer format. The chord angle parameter is in integer
	format. The sign of the parameter is ignored, except to set the maximum
	in range limit to -32768 to 32767.

	All parameters must be integers in the range -32768 to 32767.
	Specifying out of range parameters sets error 3 and the command
	is ignored */


static void arcs(int relative)
{
	HPGL_Pt p, d, center;
	float alpha, eps, ftmp;
	double phi, phi0, r;
	double SafeLinePatLen = CurrentLinePatLen;

	/* center coordinates */
	if (read_float(&p.x)){	/* No number found      */
		set_Error(2,"no first parameter");
		return;
	}

	if (read_float(&p.y)) {	/* x without y invalid! */
		set_Error(2,"no second parameter");
		return;
	}
	if (check_value(p.x,(float) MIN_INT, (float) MAX_INT) ||
		check_value(p.y,(float) MIN_INT, (float) MAX_INT)) {
		set_Error(3,"parameter out of range");
		return;
	}

	/* arc angle */
	if (read_float(&ftmp)) {	/* Invalid without angle */
		set_Error(2,"no third parameter");
		return;
	}
	if (check_value(ftmp,(float) MIN_INT, (float) MAX_INT)) {
		set_Error(3,"parameter out of range");
		return;
	}
	alpha = ftmp* M_PI / 180.0;	/* Deg-to-Rad           */

	/* chord angle */
	if( read_float(&ftmp)) {        
		eps = 5.0;	/*    so use default!   */
	}
	else {
		if (check_value(ftmp,(float) MIN_INT, (float) MAX_INT)) {
			set_Error(3,"parameter out of range");
			return;
		}
		ftmp=fabs(ftmp);
		if (ftmp < 0.5) eps= 0.5;
		else eps= ftmp;
	}

	if (ct_dist == FALSE)
		eps *= M_PI / 180.0;	/* Deg-to-Rad           */

	if (relative) {		/* Process coordinates  */
		d = p;		/* Difference vector    */
		center.x = d.x + p_last.x;
		center.y = d.y + p_last.y;
	} else {
		d.x = p.x - p_last.x;
		d.y = p.y - p_last.y;
		center.x = p.x;
		center.y = p.y;
	}

	if (((r = sqrt(d.x * d.x + d.y * d.y)) == 0.0) || (alpha == 0.0))
		return;		/* Zero radius or zero arc angle given  */

	if (ct_dist == TRUE)
		eps = 2. * acos((r - eps) / r);

	phi0 = atan2(-d.y, -d.x);

	if (CurrentLineType == LT_adaptive) {	/* Adaptive patterns:   */
		p.x = r * cos(eps);	/* A chord segment      */
		p.y = r * sin(eps);
		if (scale_flag)
			User_to_Plotter_coord(&p, &p);

		/*      Pattern length = chord length           */
		CurrentLinePatLen = HYPOT(p.x, p.y);
	}

	if (alpha > 0.0) {
		for (phi = phi0 + MIN(eps, alpha); phi < phi0 + alpha;
		     phi += eps)
			arc_increment(&center, r, phi);
		arc_increment(&center, r, phi0 + alpha);	/* to endpoint */
	} else {
		for (phi = phi0 - MIN(eps, -alpha); phi > phi0 + alpha;
		     phi -= eps)
			arc_increment(&center, r, phi);
		arc_increment(&center, r, phi0 + alpha);	/* to endpoint */
	}
	CurrentLinePatLen = SafeLinePatLen;	/* Restore */
}
/*	The radius can be a positive or negative number in integer format.
	If scaling is off the radius is in plotter units.
	The chord angle aprameter ist in integer format.
	The sign of the parameter is ignored except to set the maximum in-range
	limit to -32768 or 32767.
	Specifying out of range parameters sets error 3 and the command is ignored */

static void circles(void)
{
	HPGL_Pt p, center;
	float eps, r, ftmp;
	double phi;
	double SafeLinePatLen = CurrentLinePatLen;

	if (read_float(&ftmp)){	/* No radius found      */
		set_Error(2,"no second parameter");
		return;
	}
	if (check_value(ftmp,(float) MIN_INT, (float) MAX_INT)) {
		set_Error(3,"parameter out of range");
		return;
	}
	r= ftmp;

	/* chord angle */
	if (read_float(&ftmp)) {        
		eps = 5.0;	/*    so use default!   */
	}
	else {
		if (check_value(ftmp,(float) MIN_INT, (float) MAX_INT)) {
			set_Error(3,"parameter out of range");
			return;
		}
		ftmp=fabs(ftmp);
		if (ftmp < 0.5) eps= 0.5;
		else eps= floor(ftmp);
	}

	if (ct_dist == TRUE)
		eps = 2. * acos((r - eps) / r);
	else
		eps *= M_PI / 180.0;	/* Deg-to-Rad           */


	center = p_last;

	if (r == 0.0)		/* Zero radius given    */
		return;

	p.x = center.x + r;
	p.y = center.y;
	Pen_action_to_tmpfile(MOVE_TO, &p, scale_flag);
	if (CurrentLineType == LT_adaptive) {	/* Adaptive patterns    */
		p.x = r * cos(eps);	/* A chord segment      */
		p.y = r * sin(eps);
		if (scale_flag)
			User_to_Plotter_coord(&p, &p);

		/*      Pattern length = chord length           */
		CurrentLinePatLen = HYPOT(p.x, p.y);
	}

	for (phi = eps; phi < 2.0 * M_PI; phi += eps) {
		p.x = center.x + r * cos(phi);
		p.y = center.y + r * sin(phi);

		Pen_action_to_tmpfile(DRAW_TO, &p, scale_flag);
		
	}
	p.x = center.x + r;	/* Close circle at r * (1, 0)   */
	p.y = center.y;
	Pen_action_to_tmpfile(DRAW_TO, &p, scale_flag);

	/* draw one overlapping segment to avoid leaving gap with wide pens */
	p.x = center.x + r * cos(eps);
	p.y = center.y + r * sin(eps);

	Pen_action_to_tmpfile(DRAW_TO, &p, scale_flag);
	Pen_action_to_tmpfile(MOVE_TO, &center, scale_flag);
	CurrentLinePatLen = SafeLinePatLen;	/* Restore */
}


static void ax_ticks(int mode)
{
	HPGL_Pt p0, p1, p2;
	LineType SafeLineType = CurrentLineType;

	p0 = p1 = p2 = p_last;
/**
 ** According to the HP-GL manual,
 ** XT & YT are not affected by LT
 **/
	CurrentLineType = LT_solid;

	if (mode == 0) {	/* X tick       */
		if (scale_flag) {
			p1.y -= neg_ticklen * (P2.y - P1.y) / Q.y;
			p2.y += pos_ticklen * (P2.y - P1.y) / Q.y;
		} else {
			p1.y -= neg_ticklen * (P2.y - P1.y);
			p2.y += pos_ticklen * (P2.y - P1.y);
		}
	} else
		/* Y tick */
	{
		if (scale_flag) {
			p1.x -= neg_ticklen * (P2.x - P1.x) / Q.x;
			p2.x += pos_ticklen * (P2.x - P1.x) / Q.x;
		} else {
			p1.x -= neg_ticklen * (P2.x - P1.x);
			p2.x += pos_ticklen * (P2.x - P1.x);
		}
	}

	Pen_action_to_tmpfile(MOVE_TO, &p1, scale_flag);
	Pen_action_to_tmpfile(DRAW_TO, &p2, scale_flag);
	Pen_action_to_tmpfile(MOVE_TO, &p0, scale_flag);

	CurrentLineType = SafeLineType;
}



/**
 **	Process a single HPGL command
 **/

static void read_HPGL_cmd(void)
{
	short old_pen;
        int cmd;
	HPGL_Pt p1 = { 0., 0. }, p2 = {
	0., 0.};
	float ftmp,ftmp1,ftmp2,ftmp3,ftmp4;
	int itmp;
	float csfont;
	char ctmp;

/**
 ** Each command consists of 2 characters. We unite them here to a single int
 ** to allow for easy processing within a big switch statement:
 **/
	cmd = cmdbuf[0] << 8;
	cmd |= cmdbuf[1] &0xFF;
        cmdbuf_ptr=2;

	switch (cmd & 0xDFDF) {	/* & forces to upper case       */
  /**
   ** Commands appear in alphabetical order within each topic group
   ** except for command synonyms.
   **/
	case AA:		/* Arc Absolute                 */
		if(lost_mode) break;
		arcs(FALSE);
		tp->CR_point=Pen_pos;
		break;
	case AF:		/* NOP */
	case AH:
	case EC:
	case AP:
	case VA:
	case VN:
	case UC:
		break;
	case AR:		/* Arc Relative                 */
		if(lost_mode) break;
		arcs(TRUE);
		tp->CR_point = Pen_pos;
		break;

	case CA: /* Alternate character set CA character set number(term)
		  or CA(term)

		The character set number can be 0 through 4. A command width no
		parameter defaults to set 0. A CA command with an invalid first
		parameter will set error 3 and the command will be ignored. */

		if (read_float(&csfont)) {	/* just CA;    */
			tp->altfont = 0;
			break;
		}
		if (csfont <0.0 || csfont > 4.0) {
 			set_Error(3,"Illegal character set");
			break;
		}
		tp->altfont = (int) csfont;
		break;

	case CI: /* Circle CI radius(,chord anlge)(term)  */
		if(lost_mode) break;
		ptemp= Pen_pos;
		circles();
		Pen_pos=ptemp;
		break;

	case PA:/* Plot Absolute, syntax check see lines 
		   terminates plot relative mode */
		plot_rel=FALSE;
		lines(plot_rel);
		tp->CR_point = tp->refpoint=Pen_pos;
		break;

	case PD: /* Pen  Down , syntax check see lines 
		    lowers pen and sets pen down bit in the status byte */
		pen_down = TRUE;
                status= status | 0x01;  /* set pen down status bit */
		if(plot_rel && lost_mode) break;
		lines(plot_rel);
		tp->CR_point = tp->refpoint=Pen_pos;
		break;

	case PR: /* Plot Relative, syntax check see lines
		   switchs into plot relative mode */
		plot_rel= TRUE;
		if(plot_rel && lost_mode) break;
		lines(plot_rel);
		tp->CR_point = tp->refpoint=Pen_pos;
		break;

	case PU: /* Pen  Up, syntax check see lines
		  raises pen, clears pen down bit in status byte */
		pen_down = FALSE;
		status= status & 0xFE;  /* clear pen down status bit */
		if(plot_rel && lost_mode) break;
		lines(plot_rel); 
		tp->CR_point = tp->refpoint=Pen_pos;
		break;

	case TL: /* Tick Length TL tp(,tn)(term) 
		    or TL(term)
		  both parameters must be between -128 and 127.9999 */

		if (read_float(&ftmp)) {	/* No number found  */
			neg_ticklen = pos_ticklen = 0.005;
			return;
		}
		if (check_value(ftmp,-128,127.999)) {
			set_Error(2,"pt parameter error");
			return;
		}
		pos_ticklen = ftmp / 100.0;

		if (read_float(&ftmp)) {	/* pos, but not neg */
			neg_ticklen = 0.0;
			return;
		}
		if (check_value(ftmp,-128,127.999)) {
			set_Error(2,"pt parameter error");
			return;
		}
		neg_ticklen = ftmp / 100.0;
		break;

	case XT: /* X Tick XT(term)   */
		ax_ticks(0);
		break;

	case YT: /* Y Tick YT(term)    */
		ax_ticks(1);
		break;


	case IP: /* Input reference Points: IP P1x,P1y(,P2x, P2y)(term)  
                    or                      IP(term)
		   parameters must be in absolute plotter units.
		   Negative parameters greater than or equal to -32 768 will be set to
		   zero. Parameters outside the maximum plotting area (hw limits of the
		   paper switch but less than 32 767 will be set to the limits of the
		   plotting area. Parameters less than -32768 or greater than 32 767
		   will cause error 3 and the coordinates of P1 and P2 will not change.
		   An IP command without parameters will default P1 and P2 to the
		   values 250,279,10250,7479 regardless of the paper switch settings.
		   
		*/
                tp->width /= (P2.x - P1.x);
                tp->height /= (P2.y - P1.y);
		if (read_float(&p1.x)) {	/* No number found, IP without params  */
			P1.x = P1X_default;
			P1.y = P1Y_default;
			P2.x = P2X_default;
			P2.y = P2Y_default;
			goto IP_Exit;
		}
		if (check_value(p1.x,(float) MIN_INT, (float) MAX_INT)) break;
		if (read_float(&p1.y)){	/* x without y! */
			set_Error(2,"no second parameter");
			break;
		}
		if (check_value(p1.y, (float) MIN_INT, (float) MAX_INT)) break;

		if (read_float(&p2.x)) {	/* No number found, only P1 spcified */
			/* it is unchecked, if P2 now is outside the plotting area */
			P2.x += p1.x - P1.x;
			P2.y += p1.y - P1.y;
			P1 = p1;
			goto IP_Exit;
		}
		if (check_value(p2.x,(float) MIN_INT, (float)MAX_INT)) break;
		if (read_float(&p2.y)) { 	/* x without y! */
			set_Error(2,"no fourth parameter");
			break;
		}
		if (check_value(p2.y, (float) MIN_INT, (float) MAX_INT)) break;

		/* verified */
		if(! read_float(&ftmp)) {
			set_Error(2,"rendundant parameter");
			break;
		}

		/* if outside hw limits, set P1,P2 to hwlimits */
		if (p1.x <0 || p1.y<0 || p2.x> hwlimit.x ||
			p2.y> hwlimit.y) {
			p1.x=0;
			p2.y=0;
			p2.x= hwlimit.x;
			p2.y= hwlimit.y;
		}
		/*
			HPGL-RTL Ref Manual: if either coordinate of P1 equals
			coordinate of P2, the coordinate of P2 is incremented by 1
			plotter unit
		*/
		if (fabs(p1.x-p2.x) < 1.0) p2.x=p2.x+1.0;
		if (fabs(p1.y-p2.y) < 1.0) p2.y=p2.y+1.0;


		P1 = p1;
		P2 = p2;


	      IP_Exit:
		S1 = P1;
		S2 = P2;
		Q.x = (P2.x - P1.x) / (S2.x - S1.x);
		Q.y = (P2.y - P1.y) / (S2.y - S1.y);
		Diag_P1_P2 = HYPOT(P2.x - P1.x, P2.y - P1.y);
		CurrentLinePatLen = 0.04 * Diag_P1_P2;
		tp->width *= (P2.x - P1.x);
		tp->height *= (P2.y - P1.y);
		adjust_text_par();
                status= status | 0x02;    /* set P1,P2 changed bit */
		/* send new P1, P2 to GUI */
		printf("%d %d %d %d %d\n", CMD_P1P2, (int)floor(P1.x+0.5), (int) floor(P1.y+0.5), (int)floor(P2.x+0.5), (int) floor(P2.y+0.5));
		return;

	case IW: /* input window IW XMin,YMin,XMax,YMax(term)
                    or IW(term)
		   Parameters are always interpreted as plotter units. If no parameters
		   are included the hard clip limits are set accordingly to the paper
		   size switch. Parameters between -3278 an 0 are set to 0. Parameters
		   larger than the limits of the absoulte plotting are but less than
                   32767 are set to the hardware limits. If XMax is less then XMin or
		   YMax is less then YMin no error is set but no plotting can occur.
		    
		*/
		if (read_float(&ftmp1)) {	/* No number found  */
			XMin=0;
			YMin=0;
			XMax=(int) floor(hwlimit.x+0.5);
		        YMax=(int) floor(hwlimit.y+0.5);
			break;
		}
		if (read_float(&ftmp2)){	/* x without y! */
			set_Error(2,"no second parameter");
			return;
		}
		if (read_float(&ftmp3)){	/* No number found  */
			set_Error(2,"no third parameter");
			return;
		}
		if (read_float(&ftmp4)){	/* x without y! */
			set_Error(2,"no fourth parameter");
			return;
		}
		/* this is specified nowhere */
		if(! read_float(&ftmp)) {
			set_Error(2,"rendundant parameter");
			break;
		}
		/* clamp values */
		if(ftmp1<0.0) ftmp1=0.0;
		if(ftmp2<0.0) ftmp2=0.0;
		if(ftmp3<0.0) ftmp3=0.0;
		if(ftmp4<0.0) ftmp4=0.0;
		if (check_value(ftmp1, 0.0, (float) MAX_INT)) ftmp1=hwlimit.x;
		if (check_value(ftmp2, 0.0, (float) MAX_INT)) ftmp1=hwlimit.y;
		if (check_value(ftmp3, 0.0, (float) MAX_INT)) ftmp1=hwlimit.x;
		if (check_value(ftmp4, 0.0, (float) MAX_INT)) ftmp1=hwlimit.y;

		/* set clip window */
		XMin= (int) floor(ftmp1+0.5);
		YMin= (int) floor(ftmp2+0.5);
		XMax= (int) floor(ftmp3+0.5);
		YMax= (int) floor(ftmp4+0.5);
		break;

	case OA: /* output actual position, either clipped or non clipped. OA(term */
		/* if we have a clipped position for the pen then output this */
		if (Pen_pos_clipped.x >=0.0 && Pen_pos_clipped.y>=0.0) 
			printf("%d %d,%d,%d\n", CMD_OUTPUT, 
				(int)floor(Pen_pos_clipped.x+0.5), 
				(int) floor(Pen_pos_clipped.y+0.5), pen_down);
		else
			printf("%d %d,%d,%d\n", CMD_OUTPUT, out_last_x, out_last_y, 
				pen_down);
		break;

	case OC: /* last commanded position is Cmd_pos. OC(term) */
		if (scale_flag)	{	/* get user coordinates, output float  */
		        printf("%d %.4f,%.4f,%d\n", CMD_OUTPUT, Cmd_pos.x, Cmd_pos.y, 
				pen_down);
		}
		else {
			/* plotter coordinates, output integers */
		        printf("%d %d,%d,%d\n", CMD_OUTPUT,  (int) floor(Cmd_pos.x+0.5), 
				(int) floor(Cmd_pos.y+0.5), pen_down);
		}
		break;

	case OE: /* output error. OE(term) 
		    clears the error flag in the status byte */
		printf("%d %d\n", CMD_OUTPUT, error);
		clear_Error();
		break;

	case OD: /* output digitized point. OD(term) 
		    clears the digitized point avaialble bit in the status byte */
		printf("%d %d,%d,%d\n", CMD_OUTPUT, (int)floor(digi.x+0.5), 
			(int) floor(digi.y+0.5), pen_down);
		status= status &0xFB; /* clear Bit pos 2 */
		break;

	case OF: /* output factors. OF(term)
		    sends always constants as specified in the HP7470A manual */
		printf("%d 40,40\n", CMD_OUTPUT);
		break;

	case OP: /* Output reference Points P1,P2. OP(term)
		    clears the P1/P2 changed bit in the status byte */
		printf("%d %d,%d,%d,%d\n",CMD_OUTPUT, (int)floor(P1.x+0.5), 
			(int) floor(P1.y+0.5), (int) floor(P2.x+0.5), 
			(int) floor(P2.y+0.5));
		status=status & 0xFD; /* clear bit pos 1: P1, P2 changed flag */
		break;

	case OI: /* output identification OI(term)
		    sends always constant as specified in the HP7470 manual */
		printf("%d 7470\n",CMD_OUTPUT);
		break;

	case OO: /* output options OO(term)
		    sends always constant as specified in the HP7470 manual */
		printf("%d 0,1,0,0,1,0,0,0\n",CMD_OUTPUT);
		break;

	case OS: /* output status OS(term)
		    clear init flag in the status byte */
		printf("%d %d\n", CMD_OUTPUT,status);
		status=status & 0xF7; /* clear bit 3 */
		break;
 
	case OW: /* Output clip box OW(term) */
		printf("%d %d,%d,%d,%d\n",CMD_OUTPUT, XMin, YMin, XMax,YMax);
		break;

	case DP: /* digitize point DP(term) */
		/* send digitize point to GUI */
		printf("%d\n",CMD_DIGI_START);
		break;

	case DC: /* digitize clear DC(term) */
		/* send digitize clear to GUI */
		printf("%d\n",CMD_DIGI_CLEAR);
		break;

	case LT: /* Line Type: LT pattern number (,pattern lenght)(term)
		    or LT(term)
		The pattern number is in decimal format but is truncated to an integer.
		The parameter should be between 0 and 6. A parameter in the range 7 to
		127.9999 is ignored; the line type does not chage and no error is set.
		A parameter 128 or greater sets error 3 and the line type does not change.
		A negative parameter between 0 and -128 defaults to a solid line and no
		error is set. A parameter < -128 sets error 3 and the line type does not
		change.

		No parameter defaults to a solid line.

		When the first parameter is between 0 and 127.9999 the second
		parameter is used. Both integer and fractional parts are use.
		When this parameter is positive and less than 127.9999 the pattern
		length is set to this length. When this parameter is negative or is
		greater than or equal to 128 the previous pattern length is used and
		error 3 is set. If a pattern length parameter is not specified, a
		length of 4% is used. */

		if (read_float(&ftmp)) { 	/* just LT;     */
			CurrentLineType = LT_solid;
			break;
		}
		ftmp=floor(ftmp);
		if(check_value(ftmp,-128.0,127.9999)) {
			set_Error(3,"parameter out of range");
			break;
		}
		if((int) ftmp <0) { 
			CurrentLineType = LT_solid;
			break;
		}
		else if((int) ftmp== 0) {
			CurrentLineType = LT_plot_at;
			CurrentLinePattern = (int) ftmp;
		}
		else if ((int) ftmp <=6) {
			CurrentLineType = LT_fixed;
			CurrentLinePattern = (int) ftmp;
		}

	
		Diag_P1_P2 = HYPOT(P2.x - P1.x, P2.y - P1.y);

		if (read_float(&ftmp)){	/* no second parameter */
			CurrentLinePatLen = Diag_P1_P2*0.04;	/* Default 4% */
			break;
		}
		if(check_value(ftmp,0.0,127.9999)) {
			break;
		}
		CurrentLinePatLen = Diag_P1_P2*	ftmp/100.0; 
		break;

	case SC: /* Input Scale Points S1,S2 SC XMin,XMax,YMin,YMax)(term) 
		  or SC(term)
		When parameters are used, all four parameters are required. Decimal
		parameters in an SC command are truncated to integers. Executing SC
		without parameters turns scaling off.
		Specifying XMax= XMin or YMax= YMin or parameters less than -32768
		or greater than 32767 will turn scling off. An SC command must have
		four or no parameters. Otherwise error 2 will be generated. An SC
		command which generates an error is ignored.
		*/
		User_to_Plotter_coord(&p_last, &p_last);
		if (read_float(&p1.x)) {	/* No number found, turn scaling off */
			S1.x = P1X_default;
			S1.y = P1Y_default;
			S2.x = P2X_default;
			S2.y = P2Y_default;
			scale_flag = FALSE;
			Q.x = Q.y = 1.0;
			break;
		}
		if (read_float(&p2.x)){	/* no second parameter */
			set_Error(2,"no second parameter");
			break;
		}
		if (read_float(&p1.y)){	/* No number found  */
			set_Error(2,"no third parameter");
			break;
		}
		if (read_float(&p2.y)) {	/* x without y! */
			set_Error(2,"no fourth parameter");
			break;
		}
		/* truncate */
		p1.x=floor(p1.x);
		p1.y=floor(p1.y);
		p2.x=floor(p2.x);
		p2.y=floor(p2.y);

		/* values out of range or zero range */
		if ( check_value(p1.x,-32768.0,32767.0) ||
			check_value(p1.y,-32768.0,32767.0) ||
			check_value(p2.x,-32768.0,32767.0) ||
			check_value(p2.y,-32768.0,32767.0) ||
			(p1.x == p2.x) ||
			(p1.y == p2.y)) {

			S1.x = P1X_default;
			S1.y = P1Y_default;
			S2.x = P2X_default;
			S2.y = P2Y_default;
			scale_flag = FALSE;
			Q.x = Q.y = 1.0;
			break;
		}
		S1.x = p1.x;
		S1.y = p1.y;
		S2.x = p2.x;
		S2.y = p2.y;
		Q.x = (P2.x - P1.x) / (S2.x - S1.x);
		Q.y = (P2.y - P1.y) / (S2.y - S1.y);
		scale_flag = TRUE;
		Plotter_to_User_coord(&p_last, &p_last);
		break;

	case SP: /* Select pen SP pen number(term)
		  or SP(term)
		  A zero parameter or no parameter stores the pen
		  An even-numbered parameter selects the left pen
		  An odd-numbered parameter selects the right pen
		  Parameter less then -32768 or 32767 cause an error and the
		  pen does not change.
		*/
		old_pen = pen;
		if (read_float(&ftmp))	{/* just SP;     */
			pen = 0;
			break;
		}
		if (check_value(ftmp,(float) MIN_INT, (float) MAX_INT)) {
			set_Error(2,"illegal parameter");
			break;
		}
		ftmp=fabs(ftmp);
		itmp= (int) floor(ftmp+0.5);
		if (itmp % 2==0) pen=2;
		else pen=1;
		/* send pen change to GUI */
		if (old_pen != pen) {
			printf("%d %d\n",SET_PEN,pen);
		}
		break;

	case DF: /* Set to default DF(term)
		   no parameters are used a numeric parameter will cause error 2
		   and the instruction will not execute */
		if (!read_float(&ftmp)) {
			set_Error(2,"rendundant parameter");
			break;
		}
		reset_HPGL();
		tp->CR_point = tp->refpoint=Pen_pos;
		break;

	case IN: /* Initialize IN(term)
		   no parameters are used a numeric parameter will cause error 2
		   and the instruction will not execute */
		if (!read_float(&ftmp)) {
			set_Error(2,"rendundant parameter");
			break;
		}
		init_HPGL();
		tp->CR_point = tp->refpoint=Pen_pos;
		/* send clear to ui */
                printf("%d\n",CLEAR);
                clear_Error();
		break;

	case IM: /* input mask IM E-Mask value (,S-Mask Value (,P-Mask value))(term)
                    or IM(term)
		    An IM command without or with illegal parameters will set
                    the mask to the default value 223. The S-Mask and P-Mask
                    values as 2nd or 3rd parameter are ignored by the HP-IL
                    version of the plotter */
		if(read_float(&ftmp)) {
			mask= 223;
			return;
		}
		mask= (int) ftmp;
		if (mask < 0 || mask > 255)
		{
			mask=223;
			set_Error(3,"Bad parameter");
		}
		break;

	case CP: /* Char Plot CP n-width,n-height(term) 
		  or CP(term)
		If no parameters are specified a CP defaults to CP0,1. The parameters must
		be in range -128 ..+127.9999. */

		if(lost_mode) break;
		if (read_float(&p1.x)) {	/* No number found  */
			p1.x=0.0;
			p1.y=-1.0;
		} else {
			if (read_float(&p1.y)){	/* no second parameter */
				set_Error(2,"no second parameter");
				break;
			}
			if (!read_float(&ftmp)) {
				set_Error(2,"rendundant parameter");
				break;
			}
		}
		if (check_value(p1.x,-128.0000,127.9999) ||
			check_value(p1.y,-128.0000,127.9999)) {
			set_Error(3,"parameter out of range");
			return;
		}
		/*
			compute offset for refpoint (only changes for p1.x)
		*/
		tp->ref_offset.x= tp->chardiff.x*p1.x;
		tp->ref_offset.y= tp->chardiff.y*p1.x;
		if (fabs(tp->ref_offset.x) < 0.05) tp->ref_offset.x=0.0;
		if (fabs(tp->ref_offset.y) < 0.05) tp->ref_offset.y=0.0;
		/*
			compute offset for CR_point and Pen_pos (only changes for p1.y)
		*/
		tp->CR_offset.x= tp->linediff.x*p1.y;
		tp->CR_offset.y= tp->linediff.y*p1.y;

		tp->CR_point.x-=tp->CR_offset.x;
		tp->CR_point.y-=tp->CR_offset.y;
		Pen_pos= tp->CR_point;
		/*
		update commanded position
		*/
		Update_commanded_position(&Pen_pos,scale_flag);
		break;

      case CS: /*character set selection CS character set number (term) 
		  or CS(term)
		The character set number can be 0 through 4. A command width no
		parameter defaults to set 0. A CS command with an invalid first
		parameter will set error 3 and the command will be ignored. */

                if (read_float(&csfont)) {   /* just CS;     */
                        tp->font = 0;
			break;
		}
		if (csfont <0.0 || csfont > 4.0) {
 			set_Error(3,"Illegal character set");
			break;
		} else	{
                        tp->font = (int) csfont;
		}
                tp->stdfont = tp->font;
                break;

	case DI: /* Char plot Dir (absolute) DI run,rise(term) 
		   or DI (term)
		Run and rise are in decimal for 0 to +- 127.9999, else error3 is set
		and the instruction is ignored.  At least one parameter must be >= 0.0004, 
		else error 3 is set and the instruction is ignored.
		A DI command with no parameters will default to DI1,0.
		A DI command with only one or more than two parameters
		will set error 2 and the instruction will be ignored. */

		if (read_float(&p1.x)) {	/* No parameter found  */
			tp->dir = 0.0;
			tp->CR_point = tp->refpoint=Pen_pos;
			adjust_text_par();
			break;
		}
		if (read_float(&p1.y)){	/* no second parameter */
			set_Error(2,"no second parameter");
			break;
		}
		if ((fabs(p1.x) < 0.0004) && (fabs(p1.y) < 0.0004)) {
			set_Error(3,"both parameters are zero");
			return;
		}
		if (check_value(p1.x,-128.0000,127.9999) ||
			check_value(p2.x,-128.0000,127.9999)) {
			set_Error(3,"parameter out of range");
			return;
		}
		if (!read_float(&ftmp)) {
			set_Error(2,"rendundant parameter");
			break;
		}
		
		tp->dir = atan2(p1.y, p1.x);
		tp->CR_point = Pen_pos;
		adjust_text_par();
		break;

	case DR: /* Char plot Dir (rel P1,P2) DI run,rise(term)
		   or DI (term)
		Run and rise are in decimal for 0 to +- 127.9999, else error3 is set
		and the instruction is ignored.  At least one parameter must be >= 0.0004, 
		else error 3 is set and the instruction is ignored.
		A DR command with no parameters will default to DR1,0.
		A DR command with only one or more than two parameters
		will set error 2 and the instruction will be ignored. */

		if (read_float(&p1.x)) {	/* No number found  */
			tp->dir = 0.0;
			tp->CR_point = tp->refpoint=Pen_pos;
			adjust_text_par();
			break;
		}

		if (read_float(&p1.y)){	/* no second parameter */
			set_Error(2,"no second parameter");
			break;
		}
		if ((fabs(p1.x) < 0.0004) && (fabs(p1.y) < 0.0004)) {
			set_Error(3,"both parameters are zero");
			return;
		}
		if (check_value(p1.x,-128.0000,127.9999) ||
			check_value(p2.x,-128.0000,127.9999)) {
			set_Error(3,"parameter out of range");
			return;
		}
		if (!read_float(&ftmp)) {
			set_Error(2,"rendundant parameter");
			break;
		}
		tp->dir =
		    atan2(p1.y * (P2.y - P1.y), p1.x * (P2.x - P1.x));
		tp->CR_point = Pen_pos;
		adjust_text_par();
		break;

	case DT:/* Define string terminator DT t(term) 
		  a DT with no parameter does not establish ETX as the default terminator.
		  The ASCII control characters 0 and 27 cannot be used as label
		  terminator */
		ctmp = cmdbuf[cmdbuf_ptr];
		StrTerm= ctmp;
		StrTermSilent=0;
		break;

	case LB: /* Label string LB c..c t(term)  */
		if(lost_mode) break;
		read_string(strbuf);
		plot_string(strbuf, LB_direct, pen);
		/*
		 * Bug fix by W. Eric Norum:
		 * Update the position so that subsequent `PR's will work.
		 */
		if (scale_flag)
			Plotter_to_User_coord(&Pen_pos, &p_last);
		else
			p_last = Pen_pos;
		break;

	case SI: /* Char cell Sizes (absolute) SI width,height(term)  
		  or SI(term)
		If parameters are included, two parameters are required. They are
		interpreted as centimetres must be in decimal format and may have any
		value between -128 and 127.9999. A SI command with no parameters will
		default to the values 0.19 for width and 0.27 for height. */

		if (read_float(&tp->width)) {	/* No number found */
			tp->width = 0.19;	/* [cm], A4     */
			tp->height = 0.27;	/* [cm], A4     */
		} else {
			if (read_float(&tp->height)){	/* no second parameter */
				set_Error(2,"no second parameter");
				break;
			}
			if (check_value(tp->width,-128.0000,127.9999) ||
				check_value(tp->height,-128.0000,127.9999)) {
				set_Error(3,"parameter out of range");
				return;
			}
			if (!read_float(&ftmp)) {
				set_Error(2,"rendundant parameter");
				break;
			}
		}
		tp->width *= 400.0;	/* [cm] --> [plotter units]        */
		tp->height *= 400.0;	/* [cm] --> [plotter units]        */
		adjust_text_par();
		break;

	case SL:/* Char Slant SL slant(term) 
		  or SL(term)
		A SL without parameters defaults to SL0. Parameters following the first
		parameter are ignored. */
		
		if (read_float(&ftmp)){	/* No number found     */
			ftmp = 0.0;
		}
		if (check_value(ftmp,-128.0000,127.9999) ) {
			set_Error(3,"parameter out of range");
			break;
		}
		tp->slant= ftmp;
		adjust_text_par();
		break;

	case SM: /* Symbol Mode SM c(term)  
		  or SM(term) 
		An SM command without parameters tuns off symbol mode. When a
		parameter is present, it is limited to a single character.
		A SM command can specify any printing character (decimal values
		33 to 127). The semicolon is used only to cancel symbol mode.
		Specifying a space or any control character also cancles symbol
		mode */

		itmp=(int) cmdbuf[cmdbuf_ptr];
		cmdbuf_ptr++;
		if (itmp == 59 || itmp < 33) {
			symbol_char='\0';
		} else {
			symbol_char= (char) itmp;
		}
		break;

	case SR: /* Character  sizes (Rel P1,P2) SR widht,height(term)
		  or SR(term)
		If parameters are included, two parameters are required. They are
		interpreted as centimetres must be in decimal format and may have any
		value between -128 and 127.9999. A SR command with no parameters will
		default to the values 0.75 for width and 1.5 for height. */

		if (read_float(&tp->width)) {	/* No number found */
			tp->width = 0.75;	/* % of (P2-P1)_x    */
			tp->height = 1.5;	/* % of (P2-P1)_y    */
		} else {
			if (read_float(&tp->height)){	/* no second parameter */
				set_Error(2,"no second parameter");
				break;
			}
			if (check_value(tp->width,-128.0000,127.9999) ||
				check_value(tp->height,-128.0000,127.9999)) {
				set_Error(3,"parameter out of range");
				return;
			}
			if (!read_float(&ftmp)) {
				set_Error(2,"rendundant parameter");
				break;
			}
		}
		tp->width *= (P2.x - P1.x) / 100.0;	/* --> [pl. units]     */
		tp->height *= (P2.y - P1.y) / 100.0;
		adjust_text_par();
		break;

	case SA: /* Select designated alternate charset SA(term) */
		if (tp->altfont)
			tp->font = tp->altfont;
		else		/* Was never designated, default to 0 */
			tp->font = 0;
		break;

	case SS: /* Select designated standard character set SS(term) */
		if (tp->stdfont)
			tp->font = tp->stdfont;
		else		/* Was never designated, default to 0 */
			tp->font = 0;
		break;

	case VS: /* select velocity VS velocity(term)
		    or VS(term)
		   the velocity must be in the range of 0 to 127.999
                   the command does nothing, only the syntax is checked
                */
		if (read_float(&ftmp))	/* Just VS */
			break;
		if (check_value(ftmp,0.0,127.9999)) set_Error(2,"Illegal parameter");
                break;

	case ZY: /* pyILPER internal command: get digitized coordinates from GUI */
		if (read_float(&ftmp))
			break;
		digi.x= ftmp;
		if (read_float(&ftmp))
			break;
		digi.y= ftmp;
		status= status | 0x04; /* set Bit pos 2 */
		break;

	case ZZ: /* pyILPER internal command: get papersize from GUI */
		if (read_float(&ftmp))
			break;
		psize= (int) floor(ftmp+0.5);
		break;

	default:		/* Skip unknown HPGL command: */
		set_Error(1,"Instruction not recognized");
		if (cmd == EOF) {
		           set_Error(1,"Unexpected EOF");

		}
		break;
	}
}

void set_Error(short errnumber, const char * errmsg)

{
	/* check wheter error is masked out */
	if ((1 << (errnumber-1)) & mask) {
		/* set bit 5 in status */
		status= status | 0x20;
		error= errnumber;
                printf("%d %s\n",CMD_EMSG,errmsg);
	}
}

void clear_Error(void)
{
	/* clear bit in status */
	status= status & 0xDF;
	error=0;
}

void check_cmdbuf(void)
{
	if (cmdbuf_ptr > cmdbufsize / 2) {
		cmdbufsize *= 2;
		cmdbuf = realloc(cmdbuf, cmdbufsize);
		if (cmdbuf == NULL) {
			Eprintf("%s\n","No memory !");
			exit(ERROR);
		}
	}
}
/*
	Main program
	Read and decode status and HPGL-Command and process command
*/

int main(int argc, char * argv[])
{
	char *line= NULL;
	int i1,i2,c;
/*
	output version and initialize
*/

	printf("%s\n",VERSION);
	fflush(stdout);
	init_HPGL();
	first_init= FALSE;
	cmdbuf_ptr=0;

	while(1) {
/*
		read and decode status 
*/
		c=getchar();
		if (c== EOF) {
			break;
		}
		i1= hex_decode_table[c];
		if (i1 < 0) {
			Eprintf("%s\n","Illegal input !");
			exit(ERROR);
		}
		c=getchar();
		if (c== EOF) {
			Eprintf("%s\n","Incomplete input !");
			exit(ERROR);
		}
		i2= hex_decode_table[c];
		if (i2 < 0) {
			Eprintf("%s\n","Illegal input !");
			exit(ERROR);
		}
               	status= (i1 <<4 | i2);
/*
		skip blank
*/
		c=getchar();
		if (c!= 0x20) {
			Eprintf("%s\n","Incomplete input !");
			exit(ERROR);
		}
/*
		read and decode command
*/
		Eprintf("%s","Command: ");
		while(1) {
			c=getchar();
			if (c== EOF) {
				Eprintf("%s\n","Incomplete input !");
				exit(ERROR);
			}
			if (c== 0x0A) {
				cmdbuf[cmdbuf_ptr]='\0';
				break;
			}
			if (c== 0x20) continue;
			i1= hex_decode_table[c];
			if (i1 < 0) {
				Eprintf("%s\n","Illegal input !");
				exit(ERROR);
			}
			c=getchar();
			if (c== EOF) {
				Eprintf("%s\n","Incomplete input !");
				exit(ERROR);
			}
			i2= hex_decode_table[c];
			if (i2 < 0) {
				Eprintf("%s\n","Illegal input !");
				exit(ERROR);
			}
			cmdbuf[cmdbuf_ptr]=(char) (i1 <<4 | i2);
			if (cmdbuf[cmdbuf_ptr] < 32)
				Eprintf("(%x)",(int)cmdbuf[cmdbuf_ptr]);
			else
				Eprintf("%c",cmdbuf[cmdbuf_ptr]);
			cmdbuf_ptr++;
		}
		Eprintf("%c",'\n');
		Eprintf("Lost mode: %d\n",lost_mode);
		cmdbuf[cmdbuf_ptr]=';';
		cmdbuf_ptr++;
		check_cmdbuf();

		/* process HPGL-command */
		read_HPGL_cmd();

		/* output status, error and termchar */
		printf("%d %d %d %d\n", CMD_STATUS, status, error, (int)StrTerm);

		/* end of output */
		printf("%d\n",CMD_EOF);
		fflush(stdout);
		cmdbuf_ptr=0;
	}
	free(line);
}

/*
 clip to polygon cohen-sutherland algorithm. Code taken from en.wikipedia.org
*/

const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000

// Compute the bit code for a point (x, y) using the clip rectangle
// bounded diagonally by (xmin, ymin), and (xmax, ymax)


int ComputeOutCode(double x, double y)
{
	int code;

	code = INSIDE;          // initialised as being inside of [[clip window]]

	if (x < XMin)           // to the left of clip window
		code |= LEFT;
	else if (x > XMax)      // to the right of clip window
		code |= RIGHT;
	if (y < YMin)           // below the clip window
		code |= BOTTOM;
	else if (y > YMax)      // above the clip window
		code |= TOP;

	return code;
}

int clipline(double *x0, double *y0, double *x1, double *y1)

{
	// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
	double x=0.0, y=0.0;
        int outcodeOut;
	int accept = FALSE;
	int outcode0;
        int outcode1;

	outcode0 = ComputeOutCode(*x0, *y0);
	outcode1 = ComputeOutCode(*x1, *y1);

	while (TRUE) {
		if (!(outcode0 | outcode1)) { 	// Bitwise OR is 0. Trivially accept and 
						// get out of loop
			accept = TRUE;
			break;
		} else if (outcode0 & outcode1) { // Bitwise AND is not 0. 
						// Trivially reject and get out of loop
			break;
		} else {
			// failed both tests, so calculate the line segment to clip
			// from an outside point to an intersection with clip edge

			// At least one endpoint is outside the clip rectangle; pick it.
			outcodeOut = outcode0 ? outcode0 : outcode1;

			// Now find the intersection point;
			// use formulas y = y0 + slope * (x - x0), x = x0 + (1 / slope) * 
			// (y - y0)
			if (outcodeOut & TOP) {           // point is above the rect.
				x = *x0 + (*x1 - *x0) * (YMax - *y0) / (*y1 - *y0);
				y = YMax;
			} else if (outcodeOut & BOTTOM) { // point is below the rect.
				x = *x0 + (*x1 - *x0) * (YMin - *y0) / (*y1 - *y0);
				y = YMin;
			} else if (outcodeOut & RIGHT) {  // point is to the right of rect,
				y = *y0 + (*y1 - *y0) * (XMax - *x0) / (*x1 - *x0);
				x = XMax;
			} else if (outcodeOut & LEFT) {   // point is to the left of rect.
				y = *y0 + (*y1 - *y0) * (XMin - *x0) / (*x1 - *x0);
				x = XMin;
			}

			// Now we move outside point to intersection point to clip
			// and get ready for next pass.
			if (outcodeOut == outcode0) {
				*x0 = x;
				*y0 = y;
				outcode0 = ComputeOutCode(*x0, *y0);
			} else {
				*x1 = x;
				*y1 = y;
				outcode1 = ComputeOutCode(*x1, *y1);
			}
		}
	}
	if (accept) {
		return TRUE;
	} else 
		return FALSE;
}
/*
  Diagnostics output, enable with DEBUG=1 in emu7470.h
*/
void Eprintf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
#ifdef DEBUG
	vfprintf(stderr, fmt, ap);
#endif
	va_end(ap);
}

