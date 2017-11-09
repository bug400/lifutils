/* text75.c -- a filter to decode HP75 text files */
/* 2000 A. R. Duell, and placed under the GPL */

/* An HP75 text file consists of : 
     A 28 byte header. The meaning of this is unknown (it may be the
     HP75's internal directory entry), and this header is ignored by
     this program

      A number of lines. Each line starts with a 3 byte header. The first 2
      bytes give the line number in BCD (low byte first). The 3rd byte is the
      length of the line in binary. The header is followed by the 
      characters that make up that line. 

       The end of the file is marked by a line having the illegal line number
       0xa999. 

   This filter reads such a file on standard input and writes the decoded 
   form (as a stream-of-bytes) to stnadard output */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"

/* This is a fiddle, but it's what 0xa999 would be decoded into */
#define END_LINE 10999

/* File header size */
#define FILE_HDR 28

int print_line(int number_flag)
  {
    /* Decode and print a single line from the text file. If number_flag
       is true, then print the line number. Return true at end-of-file, 
       false otherwise */

    unsigned char hdr[3]; /* Header bytes */
    unsigned char line[256]; /* characters in the line */
    int line_no; /* Line number from the file */
    size_t chars_read; /* Actual number of characters read */
    size_t i;

    i=fread(hdr,sizeof(char),3,stdin);
    if(i!=3)
      {
        /* EOF while reading the header */
        return(1); 
      }
    else
      {
        line_no= ((hdr[1]>>4)*1000)
                +((hdr[1]&0xf)*100)
                +((hdr[0]>>4)*10)
                +(hdr[0]&0xf);
        if(line_no==END_LINE)
          {
            /* End of file marker */
            return(1);
          }
        else
          {
            if(number_flag)
              {
                /* If desired, print the line number */
                printf("%4d : ",line_no);
              }
            /* Read in the line */
            chars_read=fread(line,sizeof(char),hdr[2],stdin);
            /* And print it, followed by newline */
            fwrite(line,sizeof(char),chars_read,stdout);
            printf("\n");
            /* Did an EOF occur? */
            return(chars_read!=hdr[2]);
          }
      }
  }

void usage(void)
  {
    fprintf(stderr,"Usage:text75 [-n]\n");
    fprintf(stderr,"      -n flag to print line numbers\n");
    exit(1);
  }

int main(int argc, char **argv)
  {
    unsigned char hdr[FILE_HDR]; /* File header */
    int end_flag; /* End of file? */
    int number_flag; /* Print line numbers ? */
    int option; /* Command line option character */

    SETMODE_STDIN_BINARY;

    number_flag=0;
    optind=1;

    /* Process command line options */
    while((option=getopt(argc,argv,"n?"))!=-1)
      {
        switch(option)
          {
            case 'n' : number_flag=1;
                       break;
            case '?' : usage();
          }
      }

    /* Read in file header */
    fread(hdr,sizeof(char),FILE_HDR,stdin);
    /* And ignore it! */

    /* Read and print the lines of the file */
    do
      {
        end_flag=print_line(number_flag);
      }
    while(!end_flag);
    exit(0);
  }

