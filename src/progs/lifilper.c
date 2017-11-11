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
#include <signal.h>
#include <sys/fcntl.h>
#include <termios.h>
#include <limits.h>
#include <sys/select.h>
#include <sys/types.h>

#define DEBUG 0
#define debug_print(fmt, ...) \
   do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

static int ilfd = -1;
static int illoop = 0;
static int lasth = 0;
static int nodisplay;


void usage(void)
  {
    fprintf(stderr,
    "Usage:lifilper [-c][-h][-s][-d] lif-image-filename tty-device scope-file\n");
    fprintf(stderr,"\n");
    fprintf(stderr, "      -c assume HP82161A cassette medum size\n");
    fprintf(stderr, "         if -c is omitted then the size of a HP9114 disk is assumed\n");
    fprintf(stderr, "      -h use connection speed of 230400 instead of 115200baud\n");
    fprintf(stderr,"      -d emulate display device\n");
    fprintf(stderr,"      -s output HP-IL commands, if scope-file is omitted\n");
    fprintf(stderr,"          output goes to standard output\n");
    fprintf(stderr,"      Note: -d and -s are mutual exclusive.\n");
    exit(1);

    fprintf(stderr,"\n");
    exit(1);
  }


// ******************************************
// ILMnemo()
//
// returns the frame mnemonic
// ******************************************
static char *ILMnemo( int frame )
{
  static char stmp[9];
         char s[7];
         int  n;

  char *mnemo[] = { "DAB", "DSR", "END", "ESR", "CMD", "RDY", "IDY", "ISR", };
  char *scmd0[] = { "NUL", "GTL", "???", "???", "SDC", "PPD", "???", "???",
                    "GET", "???", "???", "???", "???", "???", "???", "ELN",
                    "NOP", "LLO", "???", "???", "DCL", "PPU", "???", "???",
                    "EAR", "???", "???", "???", "???", "???", "???", "???", };
  char *scmd9[] = { "IFC", "???", "REN", "NRE", "???", "???", "???", "???",
                    "???", "???", "AAU", "LPD", "???", "???", "???", "???", };

  n = frame & 255;
  snprintf( s, sizeof(s), "%s %02X", mnemo[frame / 256], n );
  if( (frame & 0x700) == 0x400 )
    {
    // CMD 
    switch( n / 32 )
      {
      case 0: snprintf( s, sizeof(s), "%s", scmd0[(n & 31)] );
              break;
      case 1: if( (n & 31) == 31)
                {
                snprintf( s, sizeof(s), "%s", "UNL" );
                }
              else
                {
                snprintf( s, sizeof(s), "%s %02X", "LAD", (n & 31) );
                }
              break;
      case 2: if( (n & 31) == 31 )
                {
                snprintf( s, sizeof(s), "%s", "UNT" );
                }
              else
                {
                snprintf( s, sizeof(s), "%s %02X", "TAD", (n & 31) );
                }
              break;
      case 3: snprintf( s, sizeof(s), "%s %02X", "SAD", (n & 31) );
              break;
      case 4: if( (n & 31) < 16 )
                {
                snprintf( s, sizeof(s), "%s %02X", "PPE", (n & 31) );
                }
              else
                {
                snprintf(s, sizeof(s), "%s", scmd9[(n & 15)] );
                }
              break;
      case 5: snprintf(s, sizeof(s), "%s %02X", "DDL", (n & 31) );
              break;
      case 6: snprintf(s, sizeof(s), "%s %02X", "DDT", (n & 31) );
              break;
      }
    if( s[0] == '?' )
      {
      snprintf(s, sizeof(s), "%s %02X", "CMD", n );
      }
    }
  else if( (frame & 0x700) == 0x500 )
    {
    // RDY 
    if( n < 128 )
      {
      switch( n )
        {
        case 0: snprintf( s, sizeof(s), "%s", "RFC"); break;
        case 64: snprintf( s, sizeof(s), "%s", "ETO"); break;
        case 65: snprintf( s, sizeof(s), "%s", "ETE"); break;
        case 66: snprintf( s, sizeof(s), "%s", "NRD"); break;
        case 96: snprintf( s, sizeof(s), "%s", "SDA"); break;
        case 97: snprintf( s, sizeof(s), "%s", "SST"); break;
        case 98: snprintf( s, sizeof(s), "%s", "SDI"); break;
        case 99: snprintf( s, sizeof(s), "%s", "SAI"); break;
        case 100: snprintf( s, sizeof(s), "%s", "TCT"); break;
        default: snprintf( s, sizeof(s), "%s %02X", "RDY", n ); break;
        }
      }
    else
      {
      switch( n / 32 )
        {
        case 4: snprintf( s, sizeof(s), "%s %02X", "AAD", (n & 31) ); break;
        case 5: snprintf( s, sizeof(s), "%s %02X", "AEP", (n & 31) ); break;
        case 6: snprintf( s, sizeof(s), "%s %02X", "AES", (n & 31) ); break;
        case 7: snprintf( s, sizeof(s), "%s %02X", "AMP", (n & 31) ); break;
        default: snprintf( s, sizeof(s), "%s %02X", "RDY", (n) ); break;
        }
      }
    }

  snprintf( stmp, sizeof(stmp), "%-8s", s );
  return stmp;
}

#define CON   0x496
#define COFF  0x497
#define TDIS  0x494

#define USE8BITS 1

// ******************************************
// ReadPILBox(&byt)
//
// try to read one byte from PILBox
// return 1 if read and byte read to &byt
// return 0 if timeout
// ******************************************
static int ReadPILBox( char *byt )
{
  struct timeval tv = { 0, 50000 }; // 5ms
  fd_set         pilset;
  int            rc;

  FD_ZERO( &pilset );
  FD_SET( ilfd, &pilset );
  if( 1 == (rc = select( ilfd + 1, &pilset, NULL, NULL, &tv )) )
    read( ilfd, byt, 1 );
  else
    *byt = 0;

  return rc;
}

// ******************************************
// SendFrame(frame)
//
// send a IL frame to the PILBox
// ******************************************
static void SendFrame( int frame )
{
  int lbyt, hbyt;  // low/high bytes to serial
  int n;
  char buf[2];

  // build the low and high parts
  if( ! USE8BITS )
    {
    // use 7-bit characters for maximum compatibility
    hbyt = ((frame >> 6) & 0x1F) | 0x20;
    lbyt = (frame & 0x3F) | 0x40;
    }
  else
    {
    // 8-bit format for optimum speed
    hbyt = ((frame >> 6) & 0x1E) | 0x20;
    lbyt = (frame & 0x7F) | 0x80;
    }
  if( hbyt != lasth )
    {
    // send high part if different from last one
    lasth = hbyt;
    buf[0] = hbyt; buf[1] = lbyt; n = 2;
    }
  else
    {
    // otherwise send only low part
    buf[0] = lbyt; n = 1;
    }
  write( ilfd, buf, n );
}


// ******************************************
// PILBox(byt)
//
// manage the PILBox
// (based on Emu41 ilext2 module)
// called at each received byte
// ******************************************
static void PILBox( int byt, int noscope, int fdscope )
{
  int frame; // IL frame

  if( (byt & 0xC0) == 0 )
    {
    if( byt & 0x20 )
      {
      // high byte, save it
      lasth = (int)byt & 0xFF;
      write( ilfd, "\r", 1 ); // acknowledge
      }
    }
  else
    {
    // low byte, build frame according to format
    if( byt & 0x80 )
      {
      frame = ((lasth & 0x1E) << 6) + (byt & 0x7F);
      }
    else
      {
      frame = ((lasth & 0x1F) << 6) + (byt & 0x3F);
      }

    if( !noscope )
      {
      static int   cpt = 0;
             char *mnemo;

      printf( "%-8s", mnemo = ILMnemo( frame ) );
      if( cpt++ == 0 )
//    if( cpt++ == 8 )
        {
        printf( "\n" );
        cpt = 0;
        }
      fflush( stdout );

      if( -1 != fdscope )
        {
        write( fdscope, mnemo, strlen( mnemo ) );
        write( fdscope, "\n", 1 );
        }
      }
    frame = ILhdisc( frame ); // transmit IL frame to internal disk device
    if(! nodisplay) 
       frame = ILdisplay( frame ); // transmit IL frame to internal video device

    // send returned frame to PILBox
    SendFrame( frame );
    }
}

// ******************************************
// InitPILBox(cmd)
//
// init the PIL Box
// ******************************************
static int InitPILBox( int cmd )
{
  char byt;

  lasth = 0;
  SendFrame(cmd);
  if( 1 == ReadPILBox( &byt ) )
    if( (byt & 0x3F) == (cmd & 0x3F) )
      return 0;

  return -1;
}

void DisplayStr(char n)
{
   printf("%c",n);
}

void ClrDisplay()
{
}


static void lifil_sigint( int sig )
{
  printf( "Bye...\n" );
  illoop = 0;
}

int main( int argc, char **argv )
{
  struct termios tp;
  char           byt;
  int            fdscope = -1;
  int            highspeed= 0;
  int            option;
  int            noscope=1;
  int            cassette_flag= 0;

  optind=1;
  nodisplay=1;
  while ((option=getopt(argc,argv,"hdcs?"))!=-1)
      {
        switch(option)
          {
            case 's' : noscope=0;
                       break;
            case 'c' : cassette_flag=1;
                       break;
            case 'd' : nodisplay=0;
                       break;
            case 'h' : highspeed=1;
                       break;
            case '?' : usage();
          }
      }
  if(((optind !=argc-2) && (optind != argc-3)) ||((optind== argc-3) && noscope))
  {
     usage();
  }
  if(( !nodisplay) && (!noscope)) usage();
  debug_print("LIF image %s\n",argv[optind]);
  debug_print("Serial line %s\n",argv[optind+1]);
  debug_print("Highspeed flag %d\n",highspeed);
  debug_print("noscope flag %d\n",noscope);

  if( 0 > (ilfd = open( argv[optind+1], O_RDWR )) ) {
    fprintf(stderr,"Unable to open serial line '%s'\n", argv[optind+1] );
    exit(1);
  }
  memset( &tp, 0, sizeof(tp) );
  cfmakeraw(&tp); /* RAW */
#ifdef __APPLE__
  if(highspeed) {
    cfsetispeed(&tp,B230400);
    cfsetospeed(&tp,B230400);
  } else {
    cfsetispeed(&tp,B115200);
    cfsetospeed(&tp,B115200);
  }
  tp.c_cflag = CS8 | ! ISTRIP | ! PARENB ;
  tp.c_iflag= ! ISTRIP;
#endif
#ifdef __linux__
  if(highspeed) {
    tp.c_cflag = B230400 | CS8 | ! ISTRIP | ! PARENB ;
  } else {
    tp.c_cflag = B115200 | CS8 | ! ISTRIP | ! PARENB ;
  }
  tp.c_iflag= ! ISTRIP;

#endif

  if( -1 == tcsetattr( ilfd, TCSANOW, &tp ) )
    {
    close( ilfd );
    fprintf(stderr,"Unable to set line attributes\n" );
    exit(1);
    }

  if( -1 == InitPILBox( COFF ) )
    {
    close( ilfd );
    fprintf(stderr, "No response from PILBox\n" );
    exit(1);
    }

  illoop = 1;
  signal( SIGINT, lifil_sigint );

  if( optind == argc -3 )
    {
    debug_print("scope file %s\n",argv[optind+2]);
    if( -1 == (fdscope = open( argv[optind+2], O_WRONLY | O_TRUNC | O_CREAT, 0644 )) ) {
      fprintf(stderr, "Unable to open scope log file '%s'\n", argv[optind+2] );
      exit(1);
      }
    }

  init_ilhdisc(argv[optind],cassette_flag);
  if(! nodisplay) init_ildisplay();
  while( illoop )
    {
    if( 1 == ReadPILBox( &byt ) )
      PILBox( (int)byt & 0xFF, noscope, fdscope );
    }

  if( -1 != fdscope )
    close( fdscope );

  if( -1 != ilfd )
    {
    InitPILBox( TDIS );
    close( ilfd );
    }

  return 0;
}

