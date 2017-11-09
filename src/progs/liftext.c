/* liftext.c -- a filter to process HP text (LIF1) files */
/* 2000 A. R. Duell, and placed under the GPL */

/* An HP text file, also known as a 'LIF file type 1' consists of a 
   number of records. Each record starts with a 2 byte length (high byte 
   first), and contains that number of bytes. A record length of 0xFFFF 
   indicates the end of the file. A record length of 0xFFFE indicates a 
   null record.

   Note that if the record length is odd, then an extra dummy byte is placed
   at the end of the record. Thus the total length (header+data) of any 
   record is even.

   See HP71 Software IDS volume 1, section 11.2.8.1 for further details

   This filter decodes such files into a stream-of-bytes, with the ends 
   of records being marked by \n characters. */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"

#define END_LEN 0xFFFF 
#define NULL_LEN  0xFFFE

unsigned int record_length(void)
  {
    /* Get 2 bytes from standard input, and return the record length that
       they specify. Return 0xFFFF if end-of-file occurs */

    unsigned char data[2]; /* Buffer to store the 2 bytes in */
    if(fread(data,sizeof(char),2,stdin)!=2)
      {
        /* This is the end of the physical file, so return an end-of-file
           marker */
        return(END_LEN);
      }
    else
      {
        /* Return the record length */
        return((data[0]<<8)+data[1]);
      }
  }

void copy_chars(int length)
  {
    /* copy length characters from standard input to standard output */
    int c; /* Character to copy */
    int i; /* counter */
    for(i=0; i<length; i++)
      {
        if((c=getchar())==EOF)
          {
            /* Quit at physical end of file, and let the 
               next call to record_length end the program */
            fprintf(stderr,"liftext: Unexpected end of file \n");
            break;
          }
        putchar(c);
      }
    printf("\n");
    if(length&1)
      {
        /* If the length is odd, get the dummy byte */
        c=getchar();
      }
  }

int main(int argc, char**argv)
  {
    int length;
    SETMODE_STDIN_BINARY;
    while((length=record_length())!=END_LEN)
      {
        if(length!=NULL_LEN)
          {
            /* Copy non-empty records */
            copy_chars(length);
          }
      }
    exit(0);
  }
