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
#include "lifutils.h"
#include "lif_dir_utils.h"

#define END_LEN 0xFFFF 
#define NULL_LEN  0xFFFE

void liftext_usage()
{
   fprintf(stderr,"Usage : lifutils liftext [-r] INPUTFILE [> output file]\n");
   fprintf(stderr,"        or\n");
   fprintf(stderr,"        lifutils liftext [-r] < input file [> output file]\n");
   fprintf(stderr,"        Translate a raw HP LIF text file into an ASCII text file\n");
   fprintf(stderr,"        -r skip an existing LIF header of the input file\n");
   fprintf(stderr,"\n");
}


unsigned int liftext_record_length(FILE *fp)
  {
    /* Get 2 bytes from standard input, and return the record length that
       they specify. Return 0xFFFF if end-of-file occurs */

    unsigned char data[2]; /* Buffer to store the 2 bytes in */
    if(fread(data,sizeof(char),2,fp)!=2)
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

void liftext_copy_chars(FILE * fp,int length)
  {
    /* copy length characters from standard input to standard output */
    int c; /* Character to copy */
    int i; /* counter */
    for(i=0; i<length; i++)
      {
        if((c=getc(fp))==EOF)
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
        c=getc(fp);
      }
  }

int liftext(int argc, char**argv)
  {
    int length;
    int skip_lif=0;
    FILE *fp;

    int option;

    while ((option=getopt(argc,argv,"r?"))!=-1)
      {
        switch(option)
          {
            case 'r' : skip_lif=1;
                       break;
            case '?' : liftext_usage();
                       return(RETURN_OK);
          }
      }
    if((optind!=argc) && (optind!= argc-1)) 
       {
         liftext_usage(); 
         return(RETURN_ERROR);
       }
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

     if(skip_lif) {
        if(skip_lif_header(fp,"TEXT")) {
           fclose(fp);
           fprintf(stderr,"Error: not a LIF text file\n");
           return(RETURN_ERROR);
        } 
     } 

     while((length=liftext_record_length(fp))!=END_LEN)
       {
        if(length!=NULL_LEN)
          {
            /* Copy non-empty records */
            liftext_copy_chars(fp,length);
          }
      }
    if(fp!=stdin) fclose(fp);
    return(RETURN_OK);
  }
