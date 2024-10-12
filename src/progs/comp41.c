/*
User-Code File Converter/Compiler/De-compiler/Bar-Code Generator.
Copyright (c) Leo Duran, 2000-2007.  All rights reserved.

Build environment: Microsoft Visual C++ 1.52c  16-bit compiler.
To build, run: nmake
*/

/*
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
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include "config.h"
#include "comp41.h"
#include "lif_create_entry.h"
#include "lif_dir_utils.h"
#include "lif_const.h"
#include "xrom.h"

#define MAX_ARGS 5
#define MAX_CODE 1500
#define MAX_LINE 1500
#define MEMORY_SIZE 4096

/* global flags */
int global_label = 0;
int global_count = 0;
int global_end = 0;
int line_numbers = 0;
int force_global = 0;
int source_listing = 0;
int code_listing = 0;
int create_lif = 0;



void usage(void)
  {
    fprintf(stderr,"Usage: comp41 [-n][-g] [-x xrom_name_file][-x...] input-file [lif-file name]\n");
    fprintf(stderr,"       -l ignore line numbers in text\n");
    fprintf(stderr,"       -g global for[ \"A..J\", \"a..e\" ] with quotes: [ lbl \"A\" ]\n");
    fprintf(stderr,"       -x xrom_name_file uses those names for XROM\n");
    fprintf(stderr,"       -s output source listing on standard error\n");
    fprintf(stderr,"       -h output compiled byte code, requires -s\n");
    fprintf(stderr,"       -f create lif file, requires parameter lif-filename\n");
    fprintf(stderr,"       if input-filename is omitted, the input comes from standard input\n");
    exit(1);
  }

void get_lif_filename(int optarg)
{
   
}

int main (int argc, char **argv)
{
   FILE * fp;
   char * line = NULL;
   char * line_sav= NULL;
   size_t len = 0;
   ssize_t read;
   int i,j;

   static int line_argc;
   static int code_count;
   static int source_line = 0;
   static int errflag = 0;
   static char *line_argv[ MAX_ARGS ];
   static char code_buffer[ MAX_CODE ];
   unsigned char memory[MEMORY_SIZE]; /* compiled program */
   unsigned char dir_entry[ENTRY_SIZE];
   char lif_filename[NAME_LEN+1];
   int checksum;
   static int byte_counter=0;
   static int regs;
   int option;
 
   SETMODE_STDOUT_BINARY;

  optind=1;
  init_xrom();
  while((option=getopt(argc,argv,"f:ghlsx:?"))!=-1)
    {
      switch(option)
        {
          case 'f' : create_lif=1;
                     if(check_filename(optarg,0)==0) {
                         fprintf(stderr,"illegal LIF filename\n");      
                         exit(1);
                     }
                     pad_name(optarg,lif_filename);
                     break;
          case 'g' : force_global=1;
                     break;
          case 'x' : read_xrom(optarg);
                     break;
          case 'l' : line_numbers=1;
                     break;
          case 's' : source_listing=1;
                     break;
          case 'h' : code_listing=1;
                     break;
          case '?' : usage();
         }
    }
    if((optind!=argc) && (optind!= argc-1))
      {
        usage();
      }
   if(optind== argc-1) {
      fp=fopen(argv[argc-1],"r");
   } else {
      fp=stdin;
   }

   if (fp == NULL) {
      fprintf(stderr,"Cannot open input file\n");
      exit(1);
   }
   while ((read = getline(&line, &len, fp)) != -1) {
      line_sav=line;
      line[strlen(line)-1]='\0';
      if(line[strlen(line)-1]== '\r') line[strlen(line)-1]='\0';
      source_line++;
      if(source_listing) {
         fprintf(stderr," %4.4d  %s\n",source_line,line);
      }
      if( !global_end &&
         ( line_argc = get_line_args( line_argv, &line ))) {
         if( line_argc == MAX_ARGS && strlen( line )) {
            fprintf(stderr, "Error: too many arguments[" );
            for( line_argc=0; line_argc<MAX_ARGS; ++line_argc )
                fprintf(stderr, " %s", line_argv[ line_argc ] );
            fprintf(stderr, " %s ]\n", line );
            code_count = 0;
         }
         else {
            // compile next instruction
            code_count = compile_args( code_buffer, line_argc, line_argv );
         }
         if( code_count == 0 ) {
             errflag=1;
             if(!source_listing) {
                fprintf(stderr, "error on line %d.\n", source_line );
             }
         } else {
             if(source_listing && code_listing) {
                 fprintf(stderr,"        ");
                 for(i=0;i<code_count;i++) {
                     fprintf(stderr,"%2.2X ",(unsigned char) code_buffer[i]);
                 }
                 fprintf(stderr,"\n");
             }
         
         }
         for(i=0;i<code_count;i++) {
            memory[byte_counter]=code_buffer[i];
            if(byte_counter > MEMORY_SIZE) {
               fprintf(stderr,"code buffer exceeded\n");
               exit(1);
            }
            byte_counter++;
         }
         if( global_end && source_listing) {
             fprintf(stderr, ".END. found on line %d.\n", source_line );
         }
      }
      line=line_sav;
   }
   if(fp != stdin) fclose(fp);
   if( errflag) {
      fprintf(stderr,"error(s) in compilation\n");
      exit(1);
   } else {
      if (! global_end) { /* Append End statement */
         compile_end(code_buffer,global_count);
         for(i=0;i<3;i++) {
            memory[byte_counter]=code_buffer[i];
            byte_counter++;
            if(byte_counter > MEMORY_SIZE) {
               fprintf(stderr,"code buffer exceeded\n");
               exit(1);
            }
         }
         if(source_listing) fprintf(stderr, ".END. statement appended.\n");
      }
      if(source_listing) {
         regs=byte_counter /7;
         if(regs * 7 < byte_counter) {
            ++regs;
         }
         fprintf(stderr,"\nProgram size %ld bytes, registers needed %ld\n",byte_counter,regs);
      }
      if(create_lif) {
         /* create and write directory entry */
        create_entry(dir_entry,lif_filename,0xE080,0,byte_counter+1,0);

        /* Implementation bytes for HP41 files */
        dir_entry[28]=byte_counter >> 8;
        dir_entry[29]=byte_counter & 0xff;
        dir_entry[30]=0x00;
        dir_entry[31]=0x20;
        for(i=0;i< ENTRY_SIZE; i++) {
           putchar((int) dir_entry[i]);
        }
      }
      /* write byte code to file */
      checksum=0;
      for (i=0;i< byte_counter;i++) {
         putchar(memory[i]);
         checksum+= (int) memory[i];
      }
      /* write checksum and trailer bytes to lif file */
      if(create_lif) {
         checksum= checksum & 0xff;
         putchar((int) checksum);
         j= SECTOR_SIZE- ((byte_counter+1) % SECTOR_SIZE);
         for (i=0; i<j;i++) putchar(0);
      }
   }
   exit(0);
}

int get_line_args( char *line_argv[],char  **line_ptr )
{
   char *pc;
   int i, j, count, done;

    count = 0;
    pc = *line_ptr;
    done = ( *pc == '\0' ) ? 1 : 0;
    while( !done ) {
        // ignore leading spaces
        while( *pc == '\t' || *pc == 0x20 )
            ++pc;

        // ignore comment line
        if( *pc == ';' ||
            *pc == '#' ||
            *pc == '\0' ) {
            done = 1;
        }
        else {
            // get argument
            i = 0;
            do {
                // consider quotes as a unit
                j = is_inquotes( &pc[ i++ ] );
                i += j;

                // end of line?
                if( pc[ i ] == '\0' ) {
                    done = 2;
                }
                // end of instruction?
                else if( pc[ i ] == ',' ) {
                    if( pc[ i+1 ] == '\t' ||
                        pc[ i+1 ] == 0x20 ) {
                        pc[ i ] = '\0';
                        done = 1;
                    }
                }
                // end of argument?
                else if( pc[ i ] == '\t' ||
                         pc[ i ] == 0x20 ) {
                    pc[ i ] = '\0';
                }
            } while( pc[ i ] != '\0' );

            // put argument in list
			line_argv[ count++ ] = pc;

            // point to next argument
            pc += strlen( pc );
            if( done != 2 )
                ++pc;

            // full list?
            if( count == MAX_ARGS ) {
                done = 1;
            }
        }
    }

    // update line pointer
    *line_ptr = pc;

    return( count );
}

//#define MAX_NUMERIC     16
#define MAX_NUMERIC     1500
#define MAX_DIGITS      10
#define MAX_INT         8
#define MAX_EXP         2



int get_numeric_prefix( char *numeric, char *buffer )
{
   int i;
   int error=0;
   int num_index=0;
   int num_decimal=0;
   int num_digits=0;
   int exp_entry=0;
   int exp_sign=0;
   int exp_digits=0;

    for( i=0; i<strlen( buffer ) && !error; ++i ) {
        if( exp_entry ) {
            if( buffer[ i ] == '+' ||
                buffer[ i ] == '-' ) {
                if( exp_sign || exp_digits )
                    error = 1;
                else {
                    ++exp_sign;
                    if( buffer[ i ] == '-' ) {
                        numeric[ num_index++ ] = '-';
                    }
                }
            }
            else if( buffer[ i ] >= '0' &&
                     buffer[ i ] <= '9' ) {
                if( exp_digits == MAX_EXP )
                    error = 1;
                else {
                    ++exp_digits;
                    numeric[ num_index++ ] = buffer[ i ];
                }
            }
            else {
                error = 1;
            }
        }
        else if( buffer[ i ] >= '0' &&
                 buffer[ i ] <= '9' ) {
#if 0	// 12/07/02
            if( num_decimal ) {
                if( num_digits == MAX_DIGITS )
                    error = 1;
                else {
                    ++num_digits;
                    numeric[ num_index++ ] = buffer[ i ];
                }
            }
            else {
                if( num_digits == MAX_INT )
                    error = 1;
                else {
                    ++num_digits;
                    numeric[ num_index++ ] = buffer[ i ];
                }
            }
#else
            ++num_digits;
            numeric[ num_index++ ] = buffer[ i ];
/*
            if( num_digits == MAX_DIGITS )
                error = 1;
            else {
                 ++num_digits;
                 numeric[ num_index++ ] = buffer[ i ];
            }
*/
#endif
        }
        else if( buffer[ i ] == '.' ||
            buffer[ i ] == ',' ) {
            if( num_decimal )
                error = 1;
            else {
                ++num_decimal;
                numeric[ num_index++ ] = '.';
            }
        }
        else if( buffer[ i ] == 'E' ||
            buffer[ i ] == 'e' ) {
            ++exp_entry;
            numeric[ num_index++ ] = 'E';
        }
        else if( buffer[ i ] == '+' ||
                 buffer[ i ] == '-' ) {
            if( i == 0 ) {
                if( buffer[ i ] == '-' &&
                    strlen( buffer ) > 1 ) {
                    numeric[ num_index++ ] = buffer[ i ];
                }
            }
            else if( num_digits || num_decimal ) {
                // replace decimal point
                if( !num_digits ) {
                    numeric[ num_index-1 ] = '1';
                }

                ++exp_sign;
                ++exp_entry;
                numeric[ num_index++ ] = 'E';

                // exponent sign
                if( buffer[ i ] == '-' ) {
                    numeric[ num_index++ ] = '-';
                }
            }
            else {
                error = 1;
            }
        }
        else {
            error = 1;
        }
    }

    // terminate string
    numeric[ num_index ] = '\0';

    return( !error && strlen( numeric ) );
}


int get_text_prefix( char *text, char *buffer, int *pcount )
{
   int i, j, k;
   int error=1;
   char lbuffer[ MAX_LINE ];

    // get start-quote
    j=0;
    for( i=0, k=0; i<strlen( buffer ) && k==0; ++i ) {
        if( buffer[ i ] == '\"' ||
            buffer[ i ] == '\'' ) {
            j = i;
            k = 1;
        }
    }

    // start-quote?
    if( k ) {
        // end-quote?
        if(( i = is_inquotes( &buffer[ j ] ))) {
            // make string copy
            strcpy( lbuffer, buffer );

            // remove end-quote
            lbuffer[ i+j ] = '\0';

            // append text?
            if( j ) {
                // remove start-quote
                lbuffer[ j ] = '\0';
                if( is_append( lbuffer )) {
                    lbuffer[ j ] = 0x7F;
                    --j;
                }
                else if( !is_text( lbuffer )) {
                    j = -1;
                }
            }

            // parse for esc-sequence after start-quote
            if( j >= 0 ) {
                if( parse_text( text, &lbuffer[ j+1 ], pcount )) {
                    error = 0;
                }
            }
        }
    }
    return( !error );
}


int get_alpha_postfix( char *alpha, char *buffer, int *pcount)
{
   int i;
   char lbuffer[ MAX_LINE ];

    // end-quote?
    if(( i = is_inquotes( buffer ))) {
       // make string copy
       strcpy( lbuffer, buffer );
        // remove end-quote
        lbuffer[ i ] = '\0';
       if( parse_text( alpha, &lbuffer[ 1 ], pcount )) {
           return(1);
       }
    }
    return(0);
}


int compile_num( char *code, char *num )
{
   int i,count;

    count = strlen( num );
    for( i = 0; i < count; ++i ) {
        if( num[ i ] == '-' )
            code[ i ] = 0x1C;
        else if( num[ i ] == 'E' )
            code[ i ] = 0x1B;
        else if( num[ i ] == '.' )
            code[ i ] = 0x1A;
        else {
            code[ i ] = 0x10 + num[ i ] - '0';
        }
    }

    return( count );
}


int compile_text( char *code, char *text, int count )
{
    // TEXT0..15
//  fprintf(stderr,"compile text\n");
    code[ 0 ] = 0xF0 + count;
    if( count )
        memcpy( &code[ 1 ], text, count );

    return( count + 1 );
}


int compile_alpha( char *code, char *prefix, char *alpha, int count )
{
   int j;
   int local;
   char mm, ff;

    local = is_local_label( alpha );

//  fprintf(stderr,"compile alpha %s\n",prefix);

    // LBL "alpha"
    if( strcasecmp( prefix, "LBL" ) == 0 ) {
        if( count >= MAX_ALPHA ) {
            fprintf(stderr, "Error: alpha (global) postfix[ %s \"%s\" ] too long.\n",
                     prefix, alpha );
            return( 0 );
        }
        else if( force_global || !local ) {
            code[ 0 ] = 0xC0;
            code[ 1 ] = 0x00;
            code[ 2 ] = 0xF1 + count;
            code[ 3 ] = 0x00;
            if( count )
                memcpy( &code[ 4 ], alpha, count );

            // set LABEL flag
            global_label = 1;
            global_count = 0;
            return( count + 4 );
        }
        else {
            code[ 0 ] = 0xCF;
            code[ 1 ] = local;
            return( 2 );
        }
    }

    //  GTO "alpha"
    if( strcasecmp( prefix, "GTO" ) == 0 ||
        strcasecmp( prefix, "GOTO" ) == 0 ) {
        if( force_global || !local ) {
            code[ 0 ] = 0x1D;
            code[ 1 ] = 0xF0 + count;
            if( count )
                memcpy( &code[ 2 ], alpha, count );
            return( count + 2 );
        }
        else {
            code[ 0 ] = 0xD0;
            code[ 1 ] = 0x00;
            code[ 2 ] = local;
            return( 3 );
        }
    }

    //  XEQ "alpha"
    if( strcasecmp( prefix, "XEQ" ) == 0 ) {
        if( force_global || !local ) {
            code[ 0 ] = 0x1E;
            code[ 1 ] = 0xF0 + count;
            if( count )
                memcpy( &code[ 2 ], alpha, count );
            return( count + 2 );
        }
        else {
            code[ 0 ] = 0xE0;
            code[ 1 ] = 0x00;
            code[ 2 ] = local;
            return( 3 );
        }
    }

    //  w "alpha"
    if( strcasecmp( prefix, "W" ) == 0 ) {
        code[ 0 ] = 0x1F;
        code[ 1 ] = 0xF0 + count;
        if( count )
            memcpy( &code[ 2 ], alpha, count );
        return( count + 2 );
    }

    // XROM "alpha"
    if( strcasecmp( prefix, "XROM") == 0 ) {
        j= get_xrom_by_name(to_hp41_string(alpha,count,1));
        if (j != -1)  {
           mm= (j >> 8);
           ff= j & 0xFF;
           code[ 0 ] = 0xA0 + ( mm >> 2 );
           code[ 1 ] = (( mm & 0x03 ) << 6 ) + ff;
           return( 2 );
        }
        fprintf(stderr, "Error: unrecognized alpha postfix[ %s \"%s\" ], try: [ XROM mm,ff ]\n",
                 prefix, to_hp41_string(alpha,count,1) );
        return( 0 );
    }

    fprintf(stderr, "Error: unrecognized prefix[ %s \"%s\" ]\n", prefix, alpha );
    return( 0 );
}


int compile_arg1( char *code, char *prefix )
{
   int i, j;
   char mm, ff;

//  fprintf(stderr,"compile arg1\n");
    // .END.
    if( strcasecmp( prefix, "END" ) == 0 ||
        strcasecmp( prefix, ".END." ) == 0 ) {
        compile_end( code, global_count );

        // set .END. flag
        global_end = 1;
        return( 3 );
    }

    // XROM functions
    j= get_xrom_by_name(prefix);
    if (j != -1)  {
        mm= (j >> 8);
        ff= j & 0xFF;
        code[ 0 ] = 0xA0 + ( mm >> 2 );
        code[ 1 ] = (( mm & 0x03 ) << 6 ) + ff;
        return( 2 );
    }

    // alternate-form functions
    j = sizeof( alt_fcn1 ) / sizeof( FCN );
    for( i = 0; i < j; ++ i ) {
        if( strcasecmp( prefix, alt_fcn1[ i ].prefix ) == 0 ) {
            code[ 0 ] = ( char )alt_fcn1[ i ].code;
            return( 1 );
        }
    }

    // single-byte functions
    for( i = 0x40; i <= 0x8F; ++i ) {
        if( strcasecmp( prefix, single20_8F[ i - 0x20 ] ) == 0 ) {
            code[ 0 ] = i;
            return( 1 );
        }
    }

    fprintf(stderr, "Error: unrecognized or imcomplete function[ %s ]\n", prefix );
    /*
    fprintf(stderr, "If [ %s ] is an external module function, try: [ XROM mm,ff ]\n",
            prefix );
    */
    return( 0 );
}


int compile_arg2( char *code, char *prefix, char *postfix )
{
   int i, j, k;
   long m, f;
   char mm, ff;
   char *pm, *pf, *stop;
   char lbuffer[ MAX_LINE ];
   char num_postfix[] = "0#";
   char *ppostfix = postfix;

//  fprintf(stderr,"compile arg2\n");
    // XROM mm,ff
    if( strcasecmp( prefix, "XROM") == 0 ) {
        if(( pf = strchr( postfix, ',' ))) {
            *pf++ = '\0';
            pm = postfix;
            if( strlen( pm ) && strlen( pf )) {
                for( i = 0, j = 1; i < ( int )strlen( pm ) && j; ++i )
                    j = isdigit( pm[ i ] );
                if( j ) {
                    m = strtol( pm, (char **) &stop, 10 );
                    if( m >= 0 && m <= 31 ) {
                        for( i = 0, j = 1; i < ( int )strlen( pf ) && j; ++i )
                            j = isdigit( pf[ i ] );
                        if( j ) {
                            f = strtol( pf, (char **) &stop, 10 );
                            if( f >= 0 && f <= 63 ) {
                                mm = ( unsigned char )m;
                                ff = ( unsigned char )f;
                                code[ 0 ] = 0xA0 + ( mm >> 2 );
                                code[ 1 ] = (( mm & 0x03 ) << 6 ) + ff;
                                return( 2 );
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        // add leading "0"
        if( strlen( postfix ) == 1 &&
            isdigit( postfix[ 0 ] )) {
            num_postfix[ 1 ] = postfix[ 0 ];
            ppostfix = num_postfix;
        }

        //
        // single-byte functions
        //
        strcpy( lbuffer, prefix );
        strcat( lbuffer, " " );
        strcat( lbuffer, ppostfix );

        // LBL 00..14
        for( i = 0x01; i <= 0x0F; ++i ) {
            j = i - 0x01;
            if( strcasecmp( lbuffer, single01_1C[ j ] ) == 0 ) {
                code[ 0 ] = i;
                return( 1 );
            }
        }

        // RCL 00..15, STO 00..15
        for( i = 0x20; i <= 0x3F; ++i ) {
            j = i - 0x20;
            if( strcasecmp( lbuffer, single20_8F[ j ] ) == 0 ) {
                code[ 0 ] = i;
                return( 1 );
            }
        }

        // GTO 00..14
        for( i = 0xB1; i <= 0xBF; ++i ) {
            j = i - 0xB1;
            if( strcasecmp( lbuffer, prefixB1_BF[ j ] ) == 0 ) {
                code[ 0 ] = i;
                code[ 1 ] = 0x00;
                return( 2 );
            }
        }

        //
        // mutiple-byte functions
        //
        if( is_postfix( ppostfix, &i )) {
            //
            // 2-byte functions
            //
            strcpy( lbuffer, prefix );
            strcat( lbuffer, " " );

            // RCL __..TONE __
            for( j = 0x90; j <= 0x9F; ++j ) {
                k = j - 0x90;
                if( strcasecmp( lbuffer, prefix90_9F[ k ] ) == 0 ) {
                    code[ 0 ] = j;
                    code[ 1 ] = i;
                    return( 2 );
                }
            }

            // SF __..FC? __
            for( j = 0xA8; j <= 0xAD; ++j ) {
                k = j - 0xA8;
                if( strcasecmp( lbuffer, prefixA8_AD[ k ] ) == 0 ) {
                    code[ 0 ] = j;
                    code[ 1 ] = i;
                    return( 2 );
                }
            }

            // X<> __..LBL __
            for( j = 0xCE; j <= 0xCF; ++j ) {
                k = j - 0xCE;
                if( strcasecmp( lbuffer, prefixCE_CF[ k ] ) == 0 ) {
                    code[ 0 ] = j;
                    code[ 1 ] = i;
                    return( 2 );
                }
            }

            // alternate-form functions
            k = sizeof( alt_fcn2 ) / sizeof( FCN );
            for( j = 0; j < k; ++ j ) {
                if( strcasecmp( prefix, alt_fcn2[ j ].prefix ) == 0 ) {
                    code[ 0 ] = ( char )alt_fcn2[ j ].code;
                    code[ 1 ] = i;
                    return( 2 );
                }
            }

            //
            // 3-byte functions
            //
            // GTO __
            if( strcasecmp( prefix, "GTO" ) == 0 ||
                strcasecmp( prefix, "GOTO" ) == 0 ) {
                code[ 0 ] = 0xD0;
                code[ 1 ] = 0x00;
                code[ 2 ] = i;
                return( 3 );
            }

            // XEQ __
            if( strcasecmp( prefix, "XEQ" ) == 0 ) {
                code[ 0 ] = 0xE0;
                code[ 1 ] = 0x00;
                code[ 2 ] = i;
                return( 3 );
            }
        }
    }

    fprintf(stderr, "Error: unrecognized function[ %s %s ]\n", prefix, postfix );
    return( 0 );
}


int compile_arg3( char *code, char *prefix, char *ind, char *postfix )
{
   int i, j, k;
   char lbuffer[ MAX_LINE ];
   char num_postfix[] = "0#";
   char *ppostfix = postfix;

//  fprintf(stderr,"compile arg3\n");
    // add leading "0"
    if( strlen( postfix ) == 1 &&
        isdigit( postfix[ 0 ] )) {
        num_postfix[ 1 ] = postfix[ 0 ];
        ppostfix = num_postfix;
    }

    if( strcasecmp( ind, "IND" ) == 0 &&
        is_postfix( ppostfix, &i )) {
        //
        // 2-byte functions
        //
        strcpy( lbuffer, prefix );
        strcat( lbuffer, " " );

        // RCL IND __..TONE IND __
        for( j = 0x90; j <= 0x9F; ++j ) {
            k = j - 0x90;
            if( strcasecmp( lbuffer, prefix90_9F[ k ] ) == 0 ) {
                code[ 0 ] = j;
                code[ 1 ] = i + 0x80;
                return( 2 );
            }
        }

        // SF IND __..FC? IND __
        for( j = 0xA8; j <= 0xAD; ++j ) {
            k = j - 0xA8;
            if( strcasecmp( lbuffer, prefixA8_AD[ k ] ) == 0 ) {
                code[ 0 ] = j;
                code[ 1 ] = i + 0x80;
                return( 2 );
            }
        }

        // X<> IND __..LBL IND __
        for( j = 0xCE; j <= 0xCF; ++j ) {
            k = j - 0xCE;
            if( strcasecmp( lbuffer, prefixCE_CF[ k ] ) == 0 ) {
                code[ 0 ] = j;
                code[ 1 ] = i + 0x80;
                return( 2 );
            }
        }

        // alternate-form IND functions
        k = sizeof( alt_fcn2 ) / sizeof( FCN );
        for( j = 0; j < k; ++ j ) {
            if( strcasecmp( prefix, alt_fcn2[ j ].prefix ) == 0 ) {
                code[ 0 ] = ( char )alt_fcn2[ j ].code;
                code[ 1 ] = i;
                return( 2 );
            }
        }

        // GTO IND __
        if( strcasecmp( prefix, "GTO" ) == 0 ||
            strcasecmp( prefix, "GOTO" ) == 0 ) {
            code[ 0 ] = 0xAE;
            code[ 1 ] = i;
            return( 2 );
        }

        // XEQ IND __
        if( strcasecmp( prefix, "XEQ" ) == 0 ) {
            code[ 0 ] = 0xAE;
            code[ 1 ] = i + 0x80;
            return( 2 );
        }
    }

    fprintf(stderr, "Error: unrecognized function[ %s %s %s ]\n",
             prefix, ind, postfix );
    return( 0 );
}


int compile_label( char *code, char *label, char *alpha, int count, char *key )
{
   int asn;

//  fprintf(stderr,"compile label\n");
    asn = get_key( key );
    if( asn && count && count < MAX_ALPHA &&
        strcasecmp( label, "LBL" ) == 0 ) {
        code[ 0 ] = 0xC0;
        code[ 1 ] = 0x00;
        code[ 2 ] = 0xF1 + count;
        code[ 3 ] = asn;
        memcpy( &code[ 4 ], alpha, count );
        return( count + 4 );
    }

    if( count >= MAX_ALPHA )
        fprintf(stderr, "Error: alpha (global) postfix[ %s \"%s\" %s ] too long.\n",
                 label, alpha, key );
    else
        fprintf(stderr, "Error: invalid key assignment[ %s \"%s\" %s ]\n",
                 label, alpha, key );

    return( 0 );
}


void compile_end( char *buffer, int bytes )
{
   int a, bc;

    //
    // END: [ Ca bc xx ]
    // where: a = 0..D, bc = 00..FF
    //   bc + a[ bit0 ] = #regs ( 7 bytes/reg )
    //   a[ bit3..1 ] = #remaining bytes ( up-to 6 )
    // max #bytes = 6 + ( 0x1FF * 7 ) = 3583 bytes
    // max #bytes before using a[ bit0 ] = 6 + ( 0xFF * 7 ) = 1791
    //
    // xx = 0D: non-private, unpacked
    //
//  fprintf(stderr,"compile end\n");
    if( bytes > 3583 ) {
        a = 0;
        bc = 0;
    }
    else {
        if( bytes < 1792 )
            a = 0;
        else {
            bytes -= 1792;
            a = 1;
        }
        bc = bytes / 7;
        a += ( bytes - ( bc * 7 )) * 2;
    }

    buffer[ 0 ] = 0xC0 + a;
    buffer[ 1 ] = bc;
    buffer[ 2 ] = 0x0D;
}


int is_postfix( char *postfix, int *pindex )
{
   int i, j;

   for( i = 0; i <= 127; ++i ) {
       if( strcmp( postfix, postfix00_7F[ i ] ) == 0 ) {
           *pindex = i;
           return( 1 );
       }
   }

   // case-insensitive: "F..R"
   for( i = 107; i <= 122; ++i ) {
       if( strcasecmp( postfix, postfix00_7F[ i ] ) == 0 ) {
           *pindex = i;
           return( 1 );
       }
   }

   for( i = 102, j = 0; i <= 111; ++i, ++j ) {
       if( strcmp( postfix, alt_postfix102_111[ j ] ) == 0 ) {
           *pindex = i;
           return( 1 );
       }
   }

   for( i = 117, j = 0; i <= 122; ++i, ++j ) {
       if( strcmp( postfix, alt_postfix117_122[ j ] ) == 0 ) {
           *pindex = i;
           return( 1 );
       }
   }

   if( strcmp( postfix, "~" ) == 0 ||
       strcmp( postfix, ">" ) == 0 ||
       strcmp( postfix, "|-" ) == 0 ||
       strcmp( postfix, "\\-" ) == 0 ||
       strcmp( postfix, ">-" ) == 0 ||
       strcmp( postfix, "->" ) == 0 ) {
       *pindex = 0x7A;
       return( 1 );
   }
   return( 0 );
}

int UTFcmp(unsigned char *buffer, int len, int *utf)
{
   int ret;
   ret=1;
   for (int i=0;i<len;i++) {
      if(buffer[i]!= (unsigned char) utf[i]) ret=0;
   }
   return(ret);
}

int parse_text( char *text, char *buffer, int *pcount )
{
   int i, j, k, n;
   int esc_x;

    *pcount = 0;
    k = strlen( buffer );
    if( k == 0 )
        return( 1 );

    // "~text"
    if( buffer[ 0 ] == '~' )
        buffer[ 0 ] = 0x7F;

    i = j =  n = 0;
    do {
        --k;

        if( j == 0 ) {

            // esc-sequence
            if( buffer[ i ] == '\\' && k  ) {
                j=1;
                esc_x = 0;
            }
            // append: "|-text", ">-text", "->text"
            else if( k && n == 0 &&
               (( buffer[ i ] == '|' && buffer[ i+1 ] == '-' ) ||
                ( buffer[ i ] == '>' && buffer[ i+1 ] == '-' ) ||
                ( buffer[ i ] == '-' && buffer[ i+1 ] == '>' ))) {
                text[ n++ ] = 0x7F;
                --k;
                ++i;
            }
            // append ">text" 
            else if (k && n == 0 && buffer[i] == '>') {
                text[n++] = 0x7F;
            }

            // append "UTF-8 |-"
            else if(k>1 && n == 0 && UTFcmp(buffer+i,3,(int[] ) {0xe2,0x94,0x9c}))
            {
                 text[ n++ ] = 0x7F;
                 k-=2;
                 i+=2;
            }
            // utf micro
            else if(k && UTFcmp(buffer+i,2,(int[] ) {0xce,0xbc}))
            {
                text[ n++] = 0x0c;
                --k;
                ++i;
            }
            // utf measured angle
            else if(k>1 && UTFcmp(buffer+i,3,(int[] ) {0xe2,0x88,0xa1}))
            {
                 text[ n++ ] = 0x0d;
                 k-=2;
                 i+=2;
            }
            // utf not equal
            else if(k>1 && UTFcmp(buffer+i,3,(int[] ) {0xe2,0x89,0xa0}))
            {
                 text[ n++ ] = 0x1d;
                 k-=2;
                 i+=2;
            }
            // utf sigma
            else if(k && UTFcmp(buffer+i,2,(int[] ) {0xce,0xa3}))
            {
                text[ n++] = 0x7e;
                --k;
                ++i;
            }
            // append: ">text"
            else if( k && n == 0 && buffer[ i ] == '>' ) {
                text[ n++ ] = 0x7F;
            }
            else {
                // repeat quote
                if(( buffer[ i ] == '\"' ||
                     buffer[ i ] == '\'' ) && k &&
                     buffer[ i ] == buffer[ i+1 ] ) {
                     ++i;
                     --k;
                }
                text[ n++ ] = buffer[ i ];
            }
        }
        // start of esc-sequence
        else if( j == 1 ) {
            if( isxdigit( buffer[ i ] )) {
                if( k )
                    j = 2;
                else {
                    text[ n++ ] = get_xdigit( buffer[ i ] );
                    j = 0;
                }
            }
            else if (esc_x) {
                     text[n++] = '\\';
                     if (n < MAX_ALPHA) {
                             text[n++] = 'x';
                             if (n < MAX_ALPHA)
                                     text[n++] = buffer[i];
                     }
                     j = 0;
            }
            else if( buffer[ i ] == 'x' ) {
                   esc_x=1;
            }
            else if( buffer[ i ] == 'a' ) {
                text[ n++ ] = '\a';
                j = 0;
            }
            else if( buffer[ i ] == 'b' ) {
                text[ n++ ] = '\b';
                j = 0;
            }
            else if( buffer[ i ] == 'f' ) {
                text[ n++ ] = '\f';
                j = 0;
            }
            else if( buffer[ i ] == 'n' ) {
                text[ n++ ] = '\n';
                j = 0;
            }
            else if( buffer[ i ] == 'r' ) {
                text[ n++ ] = '\r';
                j = 0;
            }
            else if( buffer[ i ] == 't' ) {
                text[ n++ ] = '\t';
                j = 0;
            }
            else if( buffer[ i ] == 'v' ) {
                text[ n++ ] = '\v';
                j = 0;
            }
            else if( buffer[ i ] == '?' ) {
                text[ n++ ] = '\?';
                j = 0;
            }
            else if( buffer[ i ] == '\"' ) {
                text[ n++ ] = '\"';
                j = 0;
            }
            else if( buffer[ i ] == '\'' ) {
                text[ n++ ] = '\'';
                j = 0;
            }
            else if( buffer[ i ] == '\\' ) {
                text[ n++ ] = '\\';
                j = 0;
            }
            else if( buffer[ i ] == '-' && n == 0 ) {
                // append: "\-text"
                text[ n++ ] = 0x7F;
                j = 0;
            }   
            else {
                text[ n++ ] = '\\';
                 if( n < MAX_ALPHA ) 
                        text[ n++ ] = buffer[ i ];
                 j = 0;
            }
        }
        // second hex-digit in esc-sequence?
        else { // j == 2
            if( isxdigit( buffer[ i ] )) {
                text[ n++ ] = ( get_xdigit( buffer[ i-1 ] ) << 4 ) +
                                get_xdigit( buffer[ i ] );
                j = 0;
            }
            else {
                text[ n++ ] = get_xdigit( buffer[ i - 1 ] );

                // start next esc-sequence?
                if( buffer[ i ] == '\\' && k ) {
                        esc_x = 0;
                        j = 1;
                }
                else {
                    if( n < MAX_ALPHA ) 
                        text[ n++ ] = buffer[ i ];
                    j = 0;
                }
            }
        }

        // overflow, underflow?
        if(( n == MAX_ALPHA && k ) || ( k == 0 && j ))
            return( 0 );

        ++i;
    } while( k );

    *pcount = n;
    return( n );
}


int is_inquotes( char *buffer )
{
   int i;
   int esc;

    esc=0;
    if( buffer[ 0 ] == '\"' || buffer[ 0 ] == '\'' ) {
        for( i=1; i<strlen( buffer ); ++i ) {
            if(buffer[i]== '\\') {
               if (esc) esc =0;
               else esc=1;
               continue;
            }
            
            if( buffer[ i ] == buffer[ 0 ] && ! esc  ) {
              if( buffer[ i+1 ] == buffer[ 0 ] )
                  ++i;
              else if( buffer[ i+1 ] == '\0' ||
                  buffer[ i+1 ] == '\t' ||
                  buffer[ i+1 ] == 0x20 ) {
                  return( i );
                }
            }
            esc=0;
        }
    }
    return( 0 );
}


int is_append( char *prefix )
{
    return( strcmp( prefix, "~" ) == 0 ||
            strcmp( prefix, ">" ) == 0 ||
            strcmp( prefix, "|-" ) == 0 ||
            strcmp( prefix, "\\-" ) == 0 ||
            strcmp( prefix, ">-" ) == 0 ||
            strcmp( prefix, "->" ) == 0 ||
            strcasecmp( prefix, "APND" ) == 0 ||
            strcasecmp( prefix, "APPND" ) == 0 ||
            strcasecmp( prefix, "APPEND" ) == 0 );
}


int is_text( char *prefix )
{
    return( strcasecmp( prefix, "T" ) == 0 ||
            strcasecmp( prefix, "TXT" ) == 0 ||
            strcasecmp( prefix, "TEXT" ) == 0 );
}


int is_local_label( char *alpha )
{
   int i;

   // "A..J"
   for( i = 0x66; i <= 0x6F; ++i ) {
       if( strcmp( alpha, postfix00_7F[ i ] ) == 0 ) {
           return( i );
       }
   }

   // "a..e"
   for( i = 0x7B; i <= 0x7F; ++i ) {
       if( strcmp( alpha, postfix00_7F[ i ] ) == 0 ) {
           return( i );
       }
   }

   return( 0 );
}


int get_key( char *key )
{
   long rc;
   int row, col, shift;
   char *pkey, *stop;
   char lbuffer[ MAX_LINE ];

    strcpy( lbuffer, key );
    if(( pkey = strchr( lbuffer, ':' ))) {
        *pkey++ = '\0';
        if( *lbuffer == '\0' ||
            strcasecmp( lbuffer, "Key" ) == 0 ) {
            rc = strtol( pkey, (char **) &stop, 10 );

            // shift key?
            if( rc >= 0 )
                shift = 0;
            else {
                shift = 8;
                rc = -rc;
            }

            //
            // row = 1..8
            // col = 1..5
            //
            if( rc <= 85 ) {
                row = rc / 10;
                col = rc - row * 10;
                if( row >= 1 && row <= 8 &&
                    col >= 1 && col <= 5 &&
                  ( row <= 3 || col < 5 ) &&
                  ( row != 3 || col != 1 )) {
                    return((( col - 1 ) << 4 ) + row + shift );
                }
            }
        }
    }

    return( 0 );
}

char get_xdigit( char xdigit )
{
    if( isdigit( xdigit ))
        return( xdigit - '0' );
    else
        return( toupper( xdigit ) - 'A' + 10 );
}


int compile_args( char *code_buffer,
                  int line_argc,
                  char *line_argv[] )
{
   static int fnumeric=0;
   int base, size, count;
   char lbuffer[ MAX_LINE ];
   char num_buffer[ MAX_NUMERIC+1 ];
   char text_buffer[ MAX_ALPHA+1 ];

	base = 0;
	if( line_numbers && isdigit( line_argv[ 0 ][ 0 ] )) {
		line_argc -= 1;
		base += 1;
	}

	count = 0;
    if( line_argc == 1 ) {
        if( get_numeric_prefix( num_buffer, line_argv[ base+0 ] )) {
            if( fnumeric ) {
                code_buffer[ 0 ] = '\0';
                count = 1 + compile_num( &code_buffer[ 1 ], num_buffer );
            }
            else {
                fnumeric = 1;
                count = compile_num( code_buffer, num_buffer );
            }
        }
        else {
            fnumeric = 0;
            if( get_text_prefix( text_buffer, line_argv[ base+0 ], &size )) {
                count = compile_text( code_buffer, text_buffer, size );
            }
            else {
                count = compile_arg1( code_buffer, line_argv[ base+0 ] );
            }
        }
    }
    else if( line_argc == 2 ) {
        // make string copy
        strcpy( lbuffer, line_argv[ base+0 ] );
        strcat( lbuffer, line_argv[ base+1 ] );

        if( get_numeric_prefix( num_buffer, lbuffer )) {
            if( fnumeric ) {
                code_buffer[ 0 ] = '\0';
                count = 1 + compile_num( &code_buffer[ 1 ], num_buffer );
            }
            else {
                fnumeric = 1;
                count = compile_num( code_buffer, num_buffer );
            }
        }
        else {
            fnumeric = 0;
            if( get_text_prefix( text_buffer, lbuffer, &size )) {
                count = compile_text( code_buffer, text_buffer, size );
            }
            else if( get_alpha_postfix( text_buffer, line_argv[ base+1 ], &size )) {
                count = compile_alpha( code_buffer, line_argv[ base+0 ],
                                       text_buffer,size );
            }
            else {
                count = compile_arg2( code_buffer, line_argv[ base+0 ],
                                      line_argv[ base+1 ] );
            }
        }
    }
    else if( line_argc == 3 ) {
        if( get_alpha_postfix( text_buffer, line_argv[ base+1 ], &size )) {
            count = compile_label( code_buffer, line_argv[ base+0 ],
                                   text_buffer, size, line_argv[ base+2 ] );
        }
        else {
            count = compile_arg3( code_buffer, line_argv[ base+0 ],
                                  line_argv[ base+1 ], line_argv[ base+2 ] );
        }
    }
    else if( line_argc == 4 ) {
        // make string copy
        strcpy( lbuffer, line_argv[ base+2 ] );
        strcat( lbuffer, line_argv[ base+3 ] );

        if( get_alpha_postfix( text_buffer, line_argv[ base+1 ], &size )) {
            count = compile_label( code_buffer, line_argv[ base+0 ],
                                   text_buffer, size, lbuffer );
        }
    }

    return( count );
}
