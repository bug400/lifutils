// ------------------------------------------------------------------------------
// ILPER 1.35 for Linux
// Copyright (c) 2008-2009  J-F Garnier
// Copyright (c) 2011-2012  Ch. Gottheimer
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ------------------------------------------------------------------------------
//                                            
// ildisp.c   HP-IL display module           
// (used in Emu41/Emu71/ILper)                
//                                            
// Based on previous personal work:           
// 1986: ILPER4 video+disc, (6502 assembler)  
// 1988: ILPER5 videoil module                
// 1993: ILPER ported on PC (8086 assembler)  
// 1997: rewriten in C and included in Emu41  
// 2008: rewriten in VB for the standalone ILPER Windows version using the PILBox!
// oct 2009: changed the AID to 46 (for general 'dumb// printer)
// 2011: ported on Linux by Ch. Gottheimer
// ------------------------------------------------------------------------------

#include <stdio.h>

void DisplayStr(char n);
void ClrDisplay();

    // HP-IL data and variables */
#define AID  46     // Accessory ID = printer
#define DEFADDR  3  // default address after AAU
static char *did = "DISPLAY\r";  // Device ID 
static int status;  // HP-IL status (always 0 here)
static int adr;     // bits 0-5 = HP-IL address, bit 7 = 1 means auto address taken 
static int fvideo;  // HP-IL state machine flags: 
    // bit 7, bit 6:
    // 0      0       idle
    // 1      0       addressed listen
    // 0      1       addressed talker 
    // 1      1       active talker
    // ( this choice comes from my original 6502 assembly code [PLTERx and ILPERx applications, 1984-1988]
    //   that used the efficient BIT opcode to test both bit 6 and 7)
static int ptsdi;   // output pointer for device ID 

// ****************************************** 
// init_ildisplay()                           
//                                            
// init the virtual HPIL display device       
// ****************************************** 
void init_ildisplay( void )
{
  status = 0;
  adr = 0;
  fvideo = 0;
  ptsdi = 0;
}

// ****************************************** 
// traite_doe(frame)                          
//                                            
// manage the HPIL data frames                
// returns the returned frame                 
// ****************************************** 
static int traite_doe( int frame )
{
  int  n;
  char c;

  n = frame & 255;
  if( fvideo & 0x80 )
    {
    // addressed 
    if( fvideo & 0x40 )
      {
      // talker 
      if( ptsdi > 0 )
        {
        c = did[ptsdi - 1];
        frame = (int)c & 0xFF;
        if( c == '\r' )
          {
          ptsdi = 0;
          }
        else
          {
          ptsdi = ptsdi + 1;
          }
        }
      if( ptsdi == 0 )
        {
        frame = 0x540;  // EOT 
        fvideo = 0x40;
        }
      }
    else
      {
      // listener 
      DisplayStr( (char)n );
      }
    }
  return frame;
}

// ****************************************** 
// traite_cmd(frame)                          
//                                            
// manage the HPIL command frames             
// returns the returned frame                 
// ****************************************** 
static int traite_cmd( int frame )
{
  int n;

  n = frame & 255;
  switch( n >> 5 )
    {
    case 0:
          switch( n )
            {
            case 4: // SDC 
                   if( fvideo & 0x80 )
                     {
                     ClrDisplay();
                     }
                   break;
            case 20: // DCL 
                    ClrDisplay();
                   break;
            }
    case 1: // LAD 
           n = n & 31;
           if( n == 31 )
             {
             // UNL, if not talker then go to idle state 
             if( (fvideo & 0x40) == 0 )
               {
               fvideo = 0;
               }
             }
           else
             {
             // else, if MLA go to listen state 
             if( n == (adr & 31) )
               {
               fvideo = 0x80;
               }
             }
           break;
    case 2: // TAD 
           n = n & 31;
           if( n == (adr & 31) )
             {
             // if MTA go to talker state 
             fvideo = 0x40;
             }
           else
             {
             // else if addressed talker, go to idle state 
             if( (fvideo & 0x40) != 0 )
               {
               fvideo = 0;
               }
             }
           break;
    case 4:
           n = n & 31;
           switch( n )
             {
             case 16: // IFC 
                     fvideo = 0;
                     break;
             case 26: // AAU 
                     adr = DEFADDR;
                     break;
             }
    }

    return frame;
}

// ******************************************
// traite_rdy(frame)                          
//                                            
// manage the HPIL ready frames               
// returns the returned frame                 
// ****************************************** 
static int traite_rdy( int frame )
{
  int n;

  n = frame & 255;
  if( n <= 127 )
    {
    // sot 
    if( fvideo & 0x40 )
      {
      // if addressed talker 
      if(n == 66)
        {
        // NRD 
        ptsdi = 0; // abort transfer 
        }
      else if( n == 96 )
        {
        // SDA 
        // no response 
        }
      else if( n == 97 )
        {
        // SST 
        frame = status;
        fvideo = 0xC0; // active talker
        }
      else if( n == 98 )
        {
        // SDI 
        frame = (int)did[0] & 0xFF;
        ptsdi = 2;
        fvideo = 0xC0;
        }
      else if( n == 99 )
        {
        // SAI 
        frame = AID;
        fvideo = 0xC0;
        }
      }
    }
  else if( n < 0x9E )
    {
    // AAD, if not already an assigned address, take it 
    if( (adr & 0x80) == 0 )
      {
      adr = n;
      frame = frame + 1;
      }
    }

  return frame;
}

// ****************************************** 
// ildisplay(frame)                           
//                                            
// manage a HPIL frame                       
// returns the returned frame                 
// ****************************************** 
int ILdisplay( int frame )
{
  if( (frame & 0x400) == 0 )
    {
    frame = traite_doe( frame );
    }
  else if( (frame & 0x700) == 0x400 )
    {
    frame = traite_cmd( frame );
    }
  else if( (frame & 0x700) == 0x500 )
    {
    frame = traite_rdy(frame);
    }
  return frame;
}

