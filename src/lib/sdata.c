/* sdata.c -- a filter to display the contents of an SDATA file */
/* 2000 A. R. Duell and placed under the GPL */

/* An SDATA file (also known as an HP41 data file) consists of 8-byte
   records. Each record can represent : 

   a) A real number. If the low nybble of the second byte is 0, then
      the record is a +ve number, while if it's 9, the record is a -ve number
      The first 3 nybbles represent the 3-digit exponent, in the order 
      middle, low, high (!). The last 6 bytes of the record store the 
      mantissa, with the last (eighth) byte containing the most significant
      digits, and the remainder in reverse byte order from there. All 
      digits are stored in BCD.

      Exponent values 0--499 represent +ve exponents 0--499, while values 
      501-999 represent -ve exponents -499 -- -1
      If the high nybble of the exponent is 0xF, then the number is either
      a NaN (if the rest of the exponent is non-zero) or infinity (if the 
      rest of the exponent is 0), whereupon the sign nybble gives the sign
      of infinity.

   b) A text string. The low nybble of the second byte is 1. Six
      Ascii characters are packed in as follows : 

      6H 6L | 5L  1 | 0 0 | 4L 5H | 3L 4H | 2L 3H | 1L 2H | X 1H 

     The nybble X is normally 0, but contains the extra bit
     used to pack the 49 bits of a BLDSPEC character into the 
     record (there are 48 bits in the 6 characters). If X and 1H are
     both 0xF, then the string was produced by RCLFLAG and the remaining 11 
     character nybbles contain the settings of flags 0-43 

     Further information can be found in the HP71 Software IDS 
     Volume 1 Section 11.2.4.2 and 11.2.7 (although the string
     format is misdocumented there). Also see the routines 
     RED41C (0x13F28) 
     N-STR  (0x14BFC)
     [Addresses given are for the 1BBBB ROM]
     in volume 3 of the HP71 Software IDS */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"
#include "lifutils.h"
#include "lif_dir_utils.h"
#include"print_41_data.h"


void sdata_usage(void)
  {
    fprintf(stderr,"Usage : lifutils sdata [-n] [-h] [-b] [-f] [-l] [-r] INPUTFILE [> output file]\n");
    fprintf(stderr,"        or\n");
    fprintf(stderr,"        lifutils sdata [-n] [-h] [-b] [-f] [-l] [-r] < input file [> output file]\n");
    fprintf(stderr,"        Interpret a raw SDATA file\n");
    fprintf(stderr,"        -n : Number the records/registers\n");
    fprintf(stderr,"        -h : Display the raw data in hex\n");
    fprintf(stderr,"        -b : Display strings as the BLDSPEC values\n");
    fprintf(stderr,"        -f : Display strings as flag settings if appropriate\n");
    fprintf(stderr,"        -l : Print 12 digit mantissas, not 10\n");
    fprintf(stderr,"        -r skip an existing LIF header of the input file\n");
    fprintf(stderr,"\n");
  }

int sdata(int argc, char **argv)
  {
    int num_flag=0 ; /* Number the records? */
    int raw_flag=0; /* Display raw data? */
    int bldspec_flag=0; /* Display BLDSPEC values? */
    int flags_flag=0; /* Attempt to display flag settings? */
    int skip_lif=0;
    int length=0; /* print long mantissas? */
    int rec_number=0; /* Current record number */
    int option; /* Current option character */
    unsigned char record[SDATA_RECORD_SIZE]; /* current record */
    FILE *fp;

    SETMODE_STDIN_BINARY;

    /* Process command line options */
    while((option=getopt(argc,argv,"nrhbfl?"))!=-1)
      {
        switch(option)
          {
            case 'n' : num_flag=1;
                       break;
            case 'h' : raw_flag=1;
                       break;
            case 'r' : skip_lif=1;
                       break;
            case 'b' : bldspec_flag=1;
                       break;
            case 'f' : flags_flag=1;
                       break;
            case 'l' : length=1;
                       break;
            case '?' : sdata_usage();
                       return(RETURN_OK);
          }
      }

     if((optind!=argc) && (optind!= argc-1))
       {
         sdata_usage();
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

    /* Read and print the records */
    while(fread(record,sizeof(char),SDATA_RECORD_SIZE,fp)==SDATA_RECORD_SIZE)
      {
        if(num_flag)
          {
            /* Print the record number */
            printf("RR%03d = ",rec_number);
          }
        rec_number++;
        if(raw_flag)
          {
            print_hex(record);
          }
        else
          {
            print_record(record,length,bldspec_flag,flags_flag);  
          }
      }
    if(fp!=stdin) fclose(fp);
    return(RETURN_OK);
  }
