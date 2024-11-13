/* regs41.c -- a filter to display descrambled registers from an HP41 file */ 
/* 2000 A. R. Duell, and placed under the GPL */

/* This is primarily a debugging tool used to analyse 'unknown' HP41
   files. It displays the raw HP41 register data from that file */

#include<stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"
#include "lifutils.h"
#include "lif_dir_utils.h"
#include "descramble_41.h"

void regs41_usage()
{
   fprintf(stderr,"Usage : lifutils regs41 [-r] INPUTFILE [> output file]\n");
   fprintf(stderr,"        or\n");
   fprintf(stderr,"        lifutils regs41 [-r] < input file [> output file]\n");
   fprintf(stderr,"        lifutils regs41 INPUTFILE [> output file]\n");
   fprintf(stderr,"        Display a HP41 raw file as registers\n");
   fprintf(stderr,"        -r skip an existing LIF header of the input file\n");

   fprintf(stderr,"\n");
}


void regs41_print_register(unsigned char *reg)
  {
    /* print the data in one register in hex, followed by newline */
    int i; /* byte counter */
    for(i=0; i<REGISTER_SIZE; i++)
      {
        printf(" %02x",reg[i]);
      }
    printf("\n");
  }

int regs41(int argc, char **argv)
  {
    unsigned char record[SDATA_RECORD_SIZE]; /* file record */
    unsigned char reg[REGISTER_SIZE]; /* corresponding HP41 register */
    int reg_number=0; /* register counter */
    int skip_lif=0;
    FILE *fp;

    int option;

    while ((option=getopt(argc,argv,"r?"))!=-1)
      {
        switch(option)
          {
            case 'r' : skip_lif=1;
                       break;
            case '?' : regs41_usage();
                       return(RETURN_OK);
          }
      }
    if((optind!=argc) && (optind!= argc-1))
       {
         regs41_usage();
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

 
    /* read in records, translate, print */
    while(fread(record,sizeof(char),SDATA_RECORD_SIZE,fp)==SDATA_RECORD_SIZE)
      {
        descramble(record,reg);
        printf("%04x :",reg_number);
        regs41_print_register(reg);
        reg_number++;
      }
    if(fp!=stdin)fclose(fp);
    return(RETURN_OK);
  }
