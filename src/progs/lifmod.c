/* lifinit.c -- Initialize a LIF image file */
/* 2014 J. Siebold , and placed under the GPL */

#include<stdio.h>
#include<unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <limits.h>
#include <modfile.h>
#ifndef _WIN32
#define O_BINARY 0
#endif


#define DEBUG 0
#define debug_print(fmt, ...) \
   do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)


void usage(void)
  {
    fprintf(stderr,
    "Usage:lifmod [-v] [-e] [-f} [-l] mod-filename\n");
    fprintf(stderr,"      lifmod [-v] [-e] [-f} [-l] mod-filename\n");
    fprintf(stderr,"\n");
    fprintf(stderr, "      -v verbose mode\n");
    fprintf(stderr, "      -f decode FAT, if any - assumes verbose mode\n");
    fprintf(stderr, "      -e extract ROM images as .rom, if any\n");
    fprintf(stderr, "      -l if -e, add ROM images as .lst, for usage with NSIM\n");
    exit(1);
  }

int main(int argc, char **argv)
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
            case '?' : usage();
                       break;
          }
      }

    /* Are the right number of names specified ? */
    if( optind != argc-1  ) 
      {
        /* No, give an error */
        usage();
      }
    strcpy(MODFileName,argv[optind]);
    if (NULL==strrchr(MODFileName,'.') ) {
        strcat(MODFileName,".mod");
    }
    if (output_mod_info(stdout,MODFileName,verbose,decodefat,NULL))
      errors++;
    if (extract && extract_roms(MODFileName,lstfornsim))
      errors++;
    if (errors)
       fprintf(stderr,"*** %d ERROR(S)\n",errors);
    exit(0);      
  }
