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
#include "lif_create_entry.h"
#include "lif_dir_utils.h"
#include "lif_const.h"

#define TRUE 1
#define FALSE 0

#define LINE_LENGTH75  255   /* max. nr. of characters in a HP-75 text file */
#define MAX_LENGTH75  65535  /* max file size of a HP-75 text file */
#define MAX_LINES75   10000  /* max number of lines in HP-75 text files */

#define DEBUG 0
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

void usage(void)
{
   fprintf(stderr,"Usage:textlif75 [-n] LIF-filename\n");
   fprintf(stderr,"      -n: use line numbers from input file\n");
   fprintf(stderr,"      (Input comes from standard input)\n");
   fprintf(stderr,"      (Output goes to standard output)(\n");
   fprintf(stderr,"\n");
   exit(1);
}

void output_line(unsigned char * line, int line_number)
{
   /* copy l bytes from line buffer to standard output */

   int i,l;
   int high, low;

   l=strlen(line);
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

void output_byte(int b)
{
    putchar(b);
}

int main(int argc, char**argv)
{
    int num_lines;   /* numver of lines in file */
    int num_characters; /* number of characters in file */
    int file_length; /* length of HP-75 text file */
    int leftover_bytes; /* to fill entire block */
    unsigned char dir_entry[ENTRY_SIZE]; /* New directory entry */
    unsigned char input_line[LINE_LENGTH75+1];   /* input line buffer */
    unsigned char ch;
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

    SETMODE_STDOUT_BINARY;

    err= FALSE;
    scan_linenumbers=FALSE;
    for(i=0;i<MAX_LINES75+1;i++) existing[i]=FALSE;

    /* command line options */
    while ((option=getopt(argc,argv,"n?"))!=-1)
      {
        switch(option)
          {
            case '?' : usage();
                       break;
            case 'n' : scan_linenumbers=TRUE;
            break;
          }
      }

    if(optind != argc -1) {
       usage();
       exit(1);
    }

    /* Check file name */
    if(check_filename(argv[optind],0)==0)
      {
        fprintf(stderr,"Illegal file name\n");
        exit(1);
      }

    /* Pad the filename with spaces */
    pad_name(argv[optind],lif_filename);

    debug_print("LIF filename: %s\n", lif_filename);

    num_lines= num_characters= 0;
    file_length= 15;
    skip= FALSE;
    prev_number=-1;

    /* copy from stdin to line buffer */

    while(TRUE) {
        c=getchar();
        if (num_characters == 0) {

            /* scan line number */
            if ( scan_linenumbers) {
                if(c== EOF) break;

                /* skip blanks */
                while (c== ' ') c=getchar();

                /* scan number */
                number_found=FALSE;
                n=0;
                while((c<='9') && (c>='0')) {
                    n=n*10+ (c - '0');
                    number_found=TRUE;
                    c=getchar();
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
                        c=getchar();
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
                exit(1);
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

    if (err) exit(1);


    /* create and write directory entry */
    create_entry(dir_entry,lif_filename,0xE052,0,file_length,0);

    /* Store implementation bytes for HP75 text files */
    dir_entry[26]=0x80;
    dir_entry[27]=0x01;
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
    output_byte(0);
    output_byte(0);

    /* file size */
    output_byte(file_length & 0xFF);
    output_byte(file_length>>8);

    /* access bits, see HP-75 internal documentation */
    output_byte(0xBE);

    /* file type 'T' */
    output_byte(0x54);

    /* creation date and time (all zero) */
    for(i=0; i<4; i++) output_byte(0x0);

    /* file name (all blank) */
    for(i=0; i<8; i++) output_byte(0x20);

    /* PCB (all zero) */
    for(i=0; i<10; i++) output_byte(0x0);

    /* now copy file content */
    for(i=0;i<num_lines;i++) {
        output_line(lines_content[i],lines_numbers[i]);
    }
    /* write eof record */
    output_byte(0x99);
    output_byte(0xA9);
    output_byte(0x02);
    output_byte(0x8A);
    output_byte(0x0E);

    /* fill up to block length, we have to add the
       HP-75 directory length to the file size */
    leftover_bytes=SECTOR_SIZE-((file_length+18)%SECTOR_SIZE);
    for(i=0;i<leftover_bytes;i++) putchar((int) 0xFF);
    exit(0);
}
