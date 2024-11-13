/* hx41rom.c -- a filter to descramble HEPAX rom files */
/* 2015 Joachim Siebold and placed under the GPL */

/*  reads a HP-41 data file in hepax format (without header) an descrambles it to an unpacked rom file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include "config.h"
#include "lifutils.h"
#include "lif_dir_utils.h"
#include "lif_const.h"

#define DEBUG 0
#define RECORD_SIZE 8

void hx41rom_usage(void)
{
   fprintf(stderr,"Usage: lifutils hx41rom [-r] INPUTFILE > output file\n");
   fprintf(stderr,"       or\n");
   fprintf(stderr,"       lifutils hx41rom [-r] < input file > output file\n");
   fprintf(stderr,"       Unscramble SDATA file containing a scrambled HEPAX rom image\n");
   fprintf(stderr,"        -r skip an existing LIF header of the input file\n");
   fprintf(stderr,"\n");
}

int hx41rom(int argc, char**argv)
  {

    unsigned char hepax_data[SCRAMBLED_ROM_RECORD_SIZE]; /* Input data bytes */
    unsigned char rom_data[RECORD_SIZE]; /* Output data bytes */
    int i;
    int skip_lif=0;
    FILE *fp;

    int option;

    while ((option=getopt(argc,argv,"r?"))!=-1)
      {
        switch(option)
          {
            case 'r' : skip_lif=1;
                       break;
            case '?' : hx41rom_usage();
                       return(RETURN_OK);
          }
      }
     if((optind!=argc) && (optind!= argc-1))
       {
         hx41rom_usage();
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
       if(skip_lif_header(fp,"SDATA")) {
          fclose(fp);
          fprintf(stderr,"Error: not a LIF HP-41 sdata file\n");
          return(RETURN_ERROR);
       }
    }

    SETMODE_STDOUT_BINARY;

    /* read in the file data, 1 record at a time */
    while(fread(hepax_data,sizeof(unsigned char),SCRAMBLED_ROM_RECORD_SIZE,fp)==
          SCRAMBLED_ROM_RECORD_SIZE) {
          /* unscramble data */
         rom_data[0]= hepax_data[0] >> 6;
         rom_data[1]= ((hepax_data[0] & 0x3F) << 2) | (hepax_data[1] >> 6); 
         rom_data[2]= (hepax_data[1] & 0x30) >> 4;
         rom_data[3]= ((hepax_data[1] & 0xF) << 4 ) | ((hepax_data[2] & 0xF0) >> 4);
         rom_data[4]= (hepax_data[2] & 0xC) >> 2;
         rom_data[5]= ((hepax_data[2] & 0x3) << 6) | ((hepax_data[3] & 0xFC) >> 2);
         rom_data[6]= (hepax_data[3] & 0x3);
         rom_data[7]= hepax_data[4];
         for(i=0; i< RECORD_SIZE ; i++) 
            putchar((int) rom_data[i]);
    }
    return(RETURN_OK);
  }
