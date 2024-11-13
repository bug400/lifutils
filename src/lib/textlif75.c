/* textlif75.c -- a filter to create HP75 text files */
/* 2000 A. R. Duell, , 2019 J. Siebold and placed under the GPL */

/* A HP75 text file consists of : 
   75no1.pdf: p 290
              p 493
   A 28 byte header with information of the HP-75 directory entry and the PCB:
   Offset Size Function
   0      2    absolute address of the file (must be zero)
   2      2    the (binary) size of the file including program control block
               but without HP-75 directory entry (essential!)
   4      1    the access bits for the file (standard text file : 0xBE)
   5      1    the ASCII name of the type of the file ('T' = 0x54)
   6      4    the date of creation (in seconds from the beginning of the
               century in 2**-14 seconds, may be all zero)
   10     8    the ASCII name of the file padded with blanks (may be all blank)
   18     10   PCB: program control block (always zero for text files)

   The remainder of the file consists of records called lines. A line
   begins with a two byte line number in BCD and is followed by the
   one byte length of the remainder of the line. 

   The lines, which may begin numbering with zero must be in increasing 
   order with no duplicates. There is a mandatory trailing line
   called end of file line. This line has number 0xA999 and is a total of five
   bytes long including the line number and length.

   See HP-75 Description and Entry Points, Page 107 and 111.

   This filter reads a textfile and creates a LIF text files with a 
   directory header */

#include <stdio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "config.h"
#include "lifutils.h"
#include "lif_create_entry.h"
#include "lif_dir_utils.h"
#include "lif_const.h"


#define LINE_LENGTH75  255   /* max. nr. of characters in a HP-75 text file */
#define MAX_LENGTH75  65535  /* max file size of a HP-75 text file */
#define MAX_LINES75   10000  /* max number of lines in HP-75 text files */

#define DEBUG 0

void textlif75_usage(void)
{
   fprintf(stderr,"Usage : lifutils textlif75 [-n] LIFFILENAME INPUTFILE > output file\n");
   fprintf(stderr,"        or\n");
   fprintf(stderr,"        lifutils textlif75 [-n] LIFFILENAME < input file > output file\n");
   fprintf(stderr,"        Convert an ASCII file to a HP75 text file\n");
   fprintf(stderr,"        -n: use line numbers from input file\n");
   fprintf(stderr,"\n");
}

void textlif75_output_line(char * line, int line_number)
{
   /* copy l bytes from line buffer to standard output */

   int i,l;
   int high, low;

   l=(int)strlen(line);
   /* output line number (4 digit BCD) */
   high= (line_number/100)%100;
   low= line_number%100;
   putchar((int) bcd(low));
   putchar((int) bcd(high));

   /* output 1 byte line length */
   debug_print("length= %d ",l);
   putchar((int) l );
   debug_print("%2.2x",(int) l);

   /* output line content */
   for(i=0;i<l;i++) {
      putchar((int) line[i]);
      debug_print("%c", line[i]);
   }
}


int textlif75(int argc, char**argv)
{
    int num_lines;   /* numver of lines in file */
    int num_characters; /* number of characters in file */
    int file_length; /* length of HP-75 text file */
    int leftover_bytes; /* to fill entire block */
    unsigned char dir_entry[ENTRY_SIZE]; /* New directory entry */
    char input_line[LINE_LENGTH75+1];   /* input line buffer */
    char *lines_content[MAX_LINES75+1];    /* line content */
    int  lines_numbers[MAX_LINES75+1];     /* line numbers */
    int existing[MAX_LINES75+1];           /* indicator for duplicates */
    char lif_filename[NAME_LEN+1];
    int option;
    int c;
    int i,n ;
    int err, skip, number_found;
    int scan_linenumbers;
    int prev_number;
    FILE *fp;


    err= FALSE;
    scan_linenumbers=FALSE;
    for(i=0;i<MAX_LINES75+1;i++) existing[i]=FALSE;

    /* command line options */
    while ((option=getopt(argc,argv,"n?"))!=-1)
      {
        switch(option)
          {
            case '?' : textlif75_usage();
                       return(RETURN_OK);
            case 'n' : scan_linenumbers=TRUE;
            break;
          }
      }

    if(argc!=2 && argc!=3) {
       textlif75_usage();
       return(RETURN_ERROR);
    }

    /* Check file name */
    if(check_filename(argv[optind],0)==0)
      {
        fprintf(stderr,"Illegal LIF file name\n");
        return(RETURN_ERROR);
      }

    /* Pad the filename with spaces */
    pad_name(argv[optind],lif_filename);
    optind++;

    debug_print("LIF filename: %s\n", lif_filename);
    if(optind== argc-1) {
        fp=fopen(argv[argc-1],"rb");
    } else {
        fp=stdin;
        SETMODE_STDIN_BINARY;
    }

    if (fp == NULL) {
       fprintf(stderr,"Error: cannot open input file\n");
       return(RETURN_ERROR);
    }

    num_lines= num_characters= 0;
    file_length= 15;
    skip= FALSE;
    prev_number=-1;

    SETMODE_STDOUT_BINARY;
    /* copy from stdin to line buffer */

    while(TRUE) {
        c=getc(fp);
        if (num_characters == 0) {

            /* scan line number */
            if ( scan_linenumbers) {
                if(c== EOF) break;

                /* skip blanks */
                while (c== ' ') c=getc(fp);

                /* scan number */
                number_found=FALSE;
                n=0;
                while((c<='9') && (c>='0')) {
                    n=n*10+ (c - '0');
                    number_found=TRUE;
                    c=getc(fp);
                }

                if(number_found) {
                
                    /* number in ascending order */
                    if (n<= prev_number) {
                        fprintf(stderr,"Line %d: line number not in ascending order\n",num_lines+1);
                        err=TRUE;
                    }
                    /* check duplicates */
                    else if (existing[n]) {
                        fprintf(stderr,"Line %d: duplicate line number\n",num_lines+1);
                        err=TRUE;
                    }
                    else if (c != ' ') {
                        fprintf(stderr,"Line %d: illegal line syntax\n",num_lines+1);
                        err=TRUE;
                    } else {
                        lines_numbers[num_lines]=n;
                        existing[n]=TRUE;
                        prev_number=n;
                        c=getc(fp);
                    }
                } else {
                    fprintf(stderr,"Line %d: illegal line number\n",num_lines+1);
                    err= TRUE;
                }
            } else lines_numbers[num_lines]= num_lines;
        }
        if ( c == EOF ) {
            break;
        }
        if ( c == '\n') {
            input_line[num_characters]='\0';
            file_length= file_length+num_characters +3;
            lines_content[num_lines]=malloc(sizeof(char)* num_characters+1);
            if(lines_content[num_lines]== (char *) NULL) {
                fprintf(stderr,"out of memory\n");
                if(fp!=stdin)fclose(fp);
                return(RETURN_ERROR);
            }
            strcpy(lines_content[num_lines],input_line);
            num_lines++;
            num_characters=0;
            skip= FALSE;
            continue;
        }
        if ((num_characters != LINE_LENGTH75) && (! skip)) {
            input_line[num_characters]=c;
            num_characters++;
        } else {
            if(! skip) fprintf(stderr,"Line %d too long\n",num_lines+1);
            err= TRUE;
            skip= TRUE;
        }
    }
    /* EOF without end of line */
    if (num_characters>0) {
        input_line[num_characters]='\0';
        file_length= file_length+num_characters +3;
        lines_content[num_lines]=malloc(sizeof(char)* num_characters+1);
        strcpy(lines_content[num_lines],input_line);
        num_lines++;
    }        
    debug_print("number of lines in input file %d\n",num_lines);
    debug_print("LIF file length %d\n",file_length);

    /* check total length */
    if (file_length > MAX_LENGTH75) {
       fprintf(stderr,"File too large\n");
       err=TRUE;
    }

    /* check number of lines */
    if (num_lines > 10000) {
       fprintf(stderr,"Number of lines exceeds 10000\n");
       err= TRUE;
    }

    if (err) {
      if(fp!=stdin)fclose(fp);
      return(RETURN_ERROR);
    }

    /* create and write directory entry */
    create_entry(dir_entry,lif_filename,0xE052,0,file_length,0);

    /* Store implementation bytes for HP75 text files */
    /* Password area */
    dir_entry[28]=0x20;
    dir_entry[29]=0x20;
    dir_entry[30]=0x20;
    dir_entry[31]=0x20;

    for(i=0;i< ENTRY_SIZE; i++) {
       putchar((int) dir_entry[i]);
    }


    debug_print("%s\n","LIF directory entry written");

    /* write HP-75 directory entry */

    /* absolute address in memory, may be zero here */
    putchar(0);
    putchar(0);

    /* file size */
    putchar(file_length & 0xFF);
    putchar(file_length>>8);

    /* access bits, see HP-75 internal documentation */
    putchar(0xBE);

    /* file type 'T' */
    putchar(0x54);

    /* creation date and time (all zero) */
    for(i=0; i<4; i++) putchar(0x0);

    /* file name (all blank) */
    for(i=0; i<8; i++) putchar(0x20);

    /* PCB (all zero) */
    for(i=0; i<10; i++) putchar(0x0);

    /* now copy file content */
    for(i=0;i<num_lines;i++) {
        textlif75_output_line(lines_content[i],lines_numbers[i]);
    }
    /* write eof record */
    putchar(0x99);
    putchar(0xA9);
    putchar(0x02);
    putchar(0x8A);
    putchar(0x0E);

    /* fill up to block length, we have to add the
       HP-75 directory length to the file size */
    leftover_bytes=SECTOR_SIZE-((file_length+18)%SECTOR_SIZE);
    for(i=0;i<leftover_bytes;i++) putchar((int) 0xFF);
    if(fp!=stdin)fclose(fp);
    return(RETURN_OK);
}
