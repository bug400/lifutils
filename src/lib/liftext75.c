/* liftext75.c -- a filter to decode HP75 text files */
/* 2000 A. R. Duell, and placed under the GPL */

/* A HP75 text file consists of : 
   A 28 byte header which is ignored by this program

   A number of lines. Each line starts with a 3 byte header. The first 2
   bytes give the line number in BCD (low byte first). The 3rd byte is the
   length of the line in binary. The header is followed by the 
   characters that make up that line. 

   The end of the file is marked by a line having the illegal line number
   0xa999. 

   This filter reads such a file on standard input and writes the decoded 
   form (as a stream-of-bytes) to stadard output */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"
#include "lifutils.h"
#include "lif_dir_utils.h"

/* This is a fiddle, but it's what 0xa999 would be decoded into */
#define END_LINE 10999

/* File header size */
#define FILE_HDR 28

int liftext75_print_line(FILE *fp,int number_flag)
  {
    /* Decode and print a single line from the text file. If number_flag
       is true, then print the line number. Return true at end-of-file, 
       false otherwise */

    unsigned char hdr[3]; /* Header bytes */
    unsigned char line[256]; /* characters in the line */
    int line_no; /* Line number from the file */
    size_t chars_read; /* Actual number of characters read */
    size_t i;

    i=fread(hdr,sizeof(char),3,fp);
    if(i!=3)
      {
        /* EOF while reading the header */
        fprintf(stderr,"premature end of file\n");
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
                printf("%4.4d ",line_no);
              }
            /* Read in the line */
            chars_read=fread(line,sizeof(char),hdr[2],fp);
            /* And print it, followed by newline */
            fwrite(line,sizeof(char),chars_read,stdout);
            printf("\n");
            /* Did an EOF occur? */
            return(chars_read!=hdr[2]);
          }
      }
  }

void liftext75_usage(void)
  {
    fprintf(stderr,"Usage : lifutils liftext75 [-n] < input file [> output file]\n");
    fprintf(stderr,"        or\n");
    fprintf(stderr,"        lifutils liftext75 [-n] INPUTFILE [> output file]\n");
    fprintf(stderr,"        Translate a raw HP-75  text file into an ASCII text file\n");
    fprintf(stderr,"        -n flag to print line numbers\n");
    fprintf(stderr,"        -r skip an existing LIF header of the input file\n");

    fprintf(stderr,"\n");
  }

int liftext75(int argc, char **argv)
  {
    unsigned char hdr[FILE_HDR]; /* File header */
    int end_flag; /* End of file? */
    int number_flag=0; /* Print line numbers ? */
    int skip_lif=0; /* Skip lif header */
    int option; /* Command line option character */
    FILE *fp;


    optind=1;

    /* Process command line options */
    while((option=getopt(argc,argv,"nr?"))!=-1)
      {
        switch(option)
          {
            case 'n' : number_flag=1;
                       break;
            case 'r' : skip_lif=1;
                       break;
            case '?' : liftext75_usage();
                       return(RETURN_OK);
          }
      }
    if((optind!=argc) && (optind!= argc-1))
       {
         liftext75_usage();
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
        if(skip_lif_header(fp,"TXT75")) {
           fclose(fp);
           fprintf(stderr,"Error: not a LIF HP-75 text file\n");
           return(RETURN_ERROR);
        } 
     } 
    /* Read in file header */
    fread(hdr,sizeof(char),FILE_HDR,fp);
    
    /* check of we have a HP-75 text file */
    if (hdr[5]!=0x54) {
        fprintf(stderr,"This is not a HP-75 text file\n");
        return(RETURN_ERROR);
    }

    /* Read and print the lines of the file */
    do
      {
        end_flag=liftext75_print_line(fp,number_flag);
      }
    while(!end_flag);
    return(RETURN_OK);
  }

