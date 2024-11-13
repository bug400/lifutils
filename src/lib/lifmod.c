/* lifmod.c -- read content of modules files */
/* 2014 J. Siebold , and placed under the GPL */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <limits.h>
#include "config.h"
#include "lifutils.h"
#include "modfile.h"

#define DEBUG 0


void lifmod_usage(void)
  {
    fprintf(stderr,"Usage : lifutils lifmod [-v] [-e] [-f} [-l]  MODULEFILE [> output file]\n");
    fprintf(stderr,"        Read contents and info of the HP-41 module file MODULEFILE\n");
    fprintf(stderr,"        -v verbose mode\n");
    fprintf(stderr,"        -f decode FAT, if any - assumes verbose mode\n");
    fprintf(stderr,"        -e extract ROM images as .rom, if any\n");
    fprintf(stderr,"        -l if -e, add ROM images as .lst, for usage with NSIM\n");
    fprintf(stderr,"\n");
  }

int lifmod(int argc, char **argv)
  {
    /* System variables */
    int option; /* Command line option character */
    int verbose=0; /* Verbose flag */
    int extract=0; /* extract flag */
    int decodefat=0; /* decode fat flag */
    int lstfornsim=0; /* lst for nsim flag */
    int errors=0; /* error counter */
    char MODFileName[PATH_MAX]={""};

    /* Process command line options */
    optind=1;
    while ((option=getopt(argc,argv,"vfel?"))!=-1)
      {
        switch(option)
          {
            case 'v':  verbose=1;
                       break;
            case 'e':  extract=1;
                       break;
            case 'f':  decodefat=1;
                       verbose=1;
                       break;
            case 'l':  lstfornsim=1;
                       break;
            case '?' : lifmod_usage();
                       return(RETURN_OK);
          }
      }

    /* Are the right number of names specified ? */
    if( optind != argc-1  ) 
      {
        /* No, give an error */
        lifmod_usage();
        return(RETURN_ERROR);      
      }
    strcpy(MODFileName,argv[optind]);
    if (NULL==strrchr(MODFileName,'.') ) {
        strcat(MODFileName,".mod");
    }
    if (output_mod_info(stdout,MODFileName,verbose,decodefat,NULL))
      errors++;
    if (extract && extract_roms(MODFileName,lstfornsim))
      errors++;
    if (errors) {
       fprintf(stderr,"*** %d ERROR(S)\n",errors);
       return(RETURN_ERROR);      
    } else return(RETURN_OK);      
  }
