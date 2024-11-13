/* lifraw.c -- a filter to remove a lif file header */
/* 2014 Joachim Siebold and placed under the GPL */

/* rawlif reads hp41 program raw files and creates a lif file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "config.h"
#include "lif_dir_utils.h"
#include "lifutils.h"

#define DEBUG 0

void lifraw_usage(void)
{
   fprintf(stderr,"Usage: lifutils lifraw INPUTFILE > output file\n");
   fprintf(stderr,"       or");
   fprintf(stderr,"       lifutils lifraw < input file > output file\n");
   fprintf(stderr,"       Remove the LIF file header from a LIF file\n");
   fprintf(stderr,"\n");
}


int lifraw(int argc, char**argv)
  {
    int byte;
    int ret;
    FILE *fp;

    int option;

    while ((option=getopt(argc,argv,"?"))!=-1)
      {
        switch(option)
          {
            case '?' : lifraw_usage();
                       return(RETURN_OK);
          }
      }
    if((optind!=argc) && (optind!= argc-1))
      {
        lifraw_usage();
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

     if(! skip_lif_header(fp, (char *) NULL)) {
        SETMODE_STDOUT_BINARY;
        while((byte=getc(fp))!=EOF) {
           putchar(byte);
        }
        ret=RETURN_OK;
     } else {
        ret=RETURN_ERROR;
     }
     if(fp != stdin) fclose(fp);
     return(ret);
  }
