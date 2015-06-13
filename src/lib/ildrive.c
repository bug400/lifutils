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
// ildrive.c    Disk drive module          
// (used in Emu41/Emu71/ILper)                
//                                            
// Based on previous personal work:           
// 1986: ILPER4 video+disc, (6502 assembler)  
// 1988: ILPER5 discil module                
// 1993: ILPER ported on PC (8086 assembler)  
// 1997: rewriten in C and included in Emu41  
// 2008: rewriten in VB for the standalone ILPER Windows version using the PILBox!
// Oct 2009: bug fix in copybuf() and exchbuf() (VB issue...)
// 2011: ported on Linux by Ch. Gottheimer
// ------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/fcntl.h>

// HP-IL data and variables */
#define AID  16    // Accessory ID = mass storage
#define DEFADDR  2 // default address after AAU

static char *did = "HDRIVE1\r"; // Device ID 
static int   status; // HP-IL status 
static int   adr;    // bits 0-5 = HP-IL address, bit 7 = 1 means auto address taken 
static int   fetat;  // HP-IL state machine flags: 
   // bit 7, bit 6:
   // 0      0       idle
   // 1      0       addressed listen
   // 0      1       addressed talker 
   // 1      1       active talker
   // bit 1: SDA, SDI
   // bit 0: SST, SDI, SAI 
static int ptsdi; // output pointer for device ID 

// disk management variables  
static int devl, devt;    // dev lstn & tlker
static int oc;            // byte pointer
static int pe;            // record pointer */
static int pe0;           
static int fpt;           // flag pointer 
static int flpwr;         // flag partial write 
static int ptout;         // pointer out 
   // 640kb floppy drive descriptor
                      // 80 tracks, 2 sides, 16 sect/track
static char lif[] = { 0, 0, 0, 80, 0, 0, 0, 2, 0, 0, 0, 16 };
static int  nbe = 2560; // number of 256-byte sectors 

   // 128Kb cassette drive descriptor would be:
   //           // 2 tracks, 1 side, 256 sect/track
   // lif[] = { 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 1, 0 };
   // nbe = 512;   // number of 256-byte sectors

static char  buf0[256];  // buffer 0 
static char  buf1[256];  // buffer 1 
static char *hdiscfile;  // path to disk image file 


// ***************************************************************** 
// hardware access functions                                         
// ***************************************************************** 


// ****************************************** 
// rrec()
//
// read one sector n° pe (256 bytes) into buf0 
// ****************************************** 
static void rrec( void )
{
  int fs;

  if( 0 == access( hdiscfile, R_OK ) )
    {
    if( -1 != (fs = open( hdiscfile, O_RDONLY )) )
      {
      if( (-1 == lseek( fs, pe * 256, SEEK_SET )) ||
          (sizeof(buf0) != read( fs, buf0, sizeof(buf0) )) )
        {
        memset( buf0, 255, sizeof(buf0) );
        }
      close( fs );
      }
    else
      { 
      status = 20;
      }
    }
}

// ****************************************** 
// wrec()
//
// write buf0 to one sector n° pe (256 bytes) 
// ****************************************** 
static void wrec( void )
{
  int fs;

  if( 0 == access( hdiscfile, W_OK ) )
    {
    if( -1 != (fs = open( hdiscfile, O_WRONLY )) )
      {
      if( (-1 == lseek( fs, pe * 256, SEEK_SET )) ||
          (sizeof(buf0) != write( fs, buf0, sizeof(buf0) )) )
        {
        status = 24;
        }
      close( fs );
      }
    else
      { 
      status = 29;
      }
    }
  }

// ****************************************** 
// format_disc()
//
// "format" a LIF image file 
// ****************************************** 
static void format_disc( void )
{
  char buf2[256];
  int   fs;

  status = 0;
  memset( buf2, 255, sizeof(buf2) );

  if( -1 != (fs = open( hdiscfile, O_WRONLY | O_TRUNC | O_CREAT, 0644 )) )
    {
    int i;

    for( i = 0; i < nbe; i++ )
      if( sizeof(buf2) != write( fs, buf2, sizeof(buf2) ) )
        {
        status = 29;
        break;
        }
    close( fs );
    }
}

// ****************************************** 
// clrdrv()                                   
//                                            
// Clear drive, reset internal pointers       
// ******************************************
static void clrdrv( void )
{
  fpt = 0; // reset DDL4 partial sequence 
  pe = 0;  // record pointer = 0 
  oc = 0;  // byte pointer = 0 
}


// ****************************************** 
// indata(n)                                  
//                                            
// receive data to disc according to DDL cmd  
// ****************************************** 
static void indata( int n )
{
  switch( devl )
    {
    case 0:
    case 2:
    case 6:
           buf0[oc] = n & 255;
           oc = oc + 1;
           if( oc > 255 )
             {
             oc = 0;
             wrec();
             pe = pe + 1;
             if( flpwr != 0 )
               {
               rrec();
               }
             }
           else
             { 
             if( n & 0x200 )
               {
               // END 
               wrec();
               if( flpwr == 0 )
                 {
                 pe = pe + 1;
                 }
               }
             }
           break;
    case 1:
           buf1[oc] = n & 255;
           oc = oc + 1;
           if( oc > 255 )
             oc = 0;
           break;
    case 3:
           oc = n & 255;
           break;
    case 4:
           n = n & 255;
           if( fpt != 0 )
             {
             pe0 = pe0 & 0xFF00;
             pe0 = pe0 | n;
             if( pe0 < nbe )
               {
               pe = pe0;
               status = 0;
               }
             else
               { 
               status = 28;
               }
             fpt = 0;
             }
           else
             { 
             pe0 = pe0 & 255;
             pe0 = pe0 | (n << 8);
             fpt = fpt - 1;
             }
    }

}

// ****************************************** 
// outdta()                                   
//                                            
// send data from disc according to DDT cmd   
// ****************************************** 
static int outdta( void )
{
  int frame;

  switch( devt )
    {
    case 0:
    case 2:
           frame = (int)buf0[oc] & 255;
           oc = oc + 1;
           if( oc > 255 )
             {
             oc = 0;
             rrec();
             pe = pe + 1;
             }
           break;
    case 1:
           frame = (int)buf1[oc] & 255;
           oc = oc + 1;
           if( oc > 255 )
             {
             oc = 0;
             devt = 15;
             }
           break;
    case 3:
           switch( ptout )
             {
             case 0: frame = pe >> 8; break;
             case 1: frame = pe & 255; break;
             case 2: frame = oc & 255; break;
             default : frame = 0x540; // EOT 
             }
           ptout = ptout + 1;
           break;
    case 6:
           if( ptout < 12 )
             {
             frame = (int)lif[ptout] & 0xFF;
             ptout = ptout + 1;
             }
           else
             { 
             frame = 0x540; // EOT 
             }
           break;
    case 7:
           switch( ptout )
             {
             case 0: frame = nbe >> 8; break;
             case 1: frame = nbe & 255; break;
             default: frame = 0x540;  // EOT 
             }
           ptout = ptout + 1;
           break;
    default: frame = 0x540; // EOT 
    }
  if( frame == 0x540 ) // EOT
    {
    fetat = 0x40; // JFG 30/11/03
    }

  return frame;
}

// ****************************************** 
// traite_doe(frame)                          
//                                            
// manage the HPIL data frames                
// returns the returned frame                 
// ****************************************** 
static int traite_doe( int frame )
{
  char c;

  if( fetat & 0x80 )
    {
    // addressed 
    if( fetat & 0x40 )
      {
      // talker 
      if( fetat & 2 )
        {
        // data (SDA) or accessory ID (SDI) 
        if( fetat & 1 )
          {
          // SDI 
          if( ptsdi )
            {
            c = did[ptsdi - 1];
            if( c == '\r' )
              {
              ptsdi = 0;
              }
            else
              { 
              ptsdi = ptsdi + 1;
              frame = c;
              }
            }
          if( ptsdi == 0 )
            {
            frame = 0x540; // EOT 
            fetat = 0x40;
            }
          }
        else
          { 
          // SDA 
          frame = outdta();
          }
        }
      else
        { 
        // end of SST, SAI 
        frame = 0x540; // EOT 
        fetat = 0x40;
        }
      }
    else
      { 
      // listener 
      indata( frame );
      }
    }

  return frame;
}

// ****************************************** 
// copybuf()                                  
//                                            
// copy buffer 0 to buffer 1                  
// ****************************************** 
static void copybuf( void )
{
  oc = 0;
  memcpy( buf1, buf0, sizeof(buf1) );
}

// ****************************************** 
// exchbuf()                                  
//                                            
// exchange buffers 0 and 1                   
// ****************************************** 
static void exchbuf( void )
{
  char tmp[256];

  oc = 0;
  memcpy( tmp, buf1, sizeof(tmp) );
  memcpy( buf1, buf0, sizeof(buf1) );
  memcpy( buf0, tmp, sizeof(buf0) );
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
                    if( fetat & 0x80)
                      {
                      clrdrv();
                      }
                    break;
             case 20: // DCL 
                    clrdrv();
                    break;
             }
           break;
    case 1: //LAD 
           n = n & 31;
           if( n == 31 )
             {
             // UNL, if not talker then go to idle state 
             if( (fetat & 0x40) == 0 )
               {
               fetat = 0;
               }
             }
           else
             { 
             // else, if MLA go to listen state 
             if( n == (adr & 31) )
               {
               fetat = 0x80;
               }
             }
           break;
    case 2: // TAD 
           n = n & 31;
           if( n == (adr & 31) )
             {
             // if MTA go to talker state 
             fetat = 0x40;
             }
           else
             { 
             // else if addressed talker, go to idle state 
             if( (fetat & 0x40) != 0 )
               {
               fetat = 0;
               }
             }
           break;
    case 4:
           n = n & 31;
           switch( n )
             {
             case 16: // IFC 
                     fetat = 0;
                     break;
             case 26: // AAU 
                     adr = DEFADDR;
                     break;
             }
           break;
    case 5: // DDL 
           n = n & 31;
           if( (fetat & 0xC0) == 0x80 )
             {
             devl = n;
             switch( n )
               {
               case 1: flpwr = 0; break;
               case 2: oc = 0; flpwr = 0; break;
               case 4: flpwr = 0; fpt = 0; break;// 01/02/03 
               case 5: format_disc(); break;
               case 6: flpwr = 0x80; rrec(); break;
               case 7: fpt = 0; pe = 0; oc = 0; break;
               case 8: wrec(); if( flpwr == 0 ) { pe = pe + 1; } break;
               case 9: copybuf(); break;
               case 10: exchbuf(); break;
               }
             }
           break;
    case 6: // DDT 
           n = n & 31;
           if( (fetat & 0x40) == 0x40 )
             {
             devt = n;
             ptout = 0;
             switch( n )
               {
               case 0: flpwr = 0; break;
               case 2: rrec(); oc = 0; flpwr = 0; pe = pe + 1; break;
               case 4: exchbuf(); break;
               }
             }
           break;
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
    // SOT 
    if( fetat & 0x40 )
      {
      // if addressed talker 
      if( n == 66 )
        {
        // NRD 
        fetat = fetat & 0xFD; // abort transfer 
        }
      else if( n == 96 )
        {
        // SDA 
        fetat = 0xC2; // moved before outdta JFG 30/11/03 
        frame = outdta();
        }
      else if( n == 97 )
        {
        // SST 
        frame = status;
        status = status & 0xE0;
        fetat = 0xC1; // active talker (single byte status) 
        }
      else if( n == 98 )
        {
        // SDI 
        frame = did[0];
        ptsdi = 2;
        fetat = 0xC3; // active talker (multiple byte status) 
        }
      else if( n == 99 )
        {
        // SAI 
        frame = AID;
        fetat = 0xC1; // active talker (single byte status) 
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
// init_ilhdisc()                           
//                                            
// init the virtual HPIL drive device       
// ****************************************** 
void init_ilhdisc( char *filename )
{
  if( filename && *filename )
    {
    hdiscfile = filename;
    }
  else
    {
    hdiscfile = "HDRIVE1.DAT";
    }
  status = 0;
  adr = 0;
  fetat = 0;
  ptsdi = 0;
}

// ****************************************** 
// ILhdisc(frame)                           
//                                            
// manage a HPIL frame                       
// returns the returned frame                 
// ****************************************** 
int ILhdisc( int frame )
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
    frame = traite_rdy( frame );
    }
  return frame;
}

