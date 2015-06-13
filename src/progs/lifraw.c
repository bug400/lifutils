/* lifraw.c -- a filter to remove a lif file header */
/* 2014 Joachim Siebold and placed under the GPL */

/* rawlif reads hp41 program raw files and creates a lif file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define DEBUG 0
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

void usage(void)
{
   fprintf(stderr,
   "Usage: lifraw \n");
   fprintf(stderr,"      (Input comes from standard input)\n");
   fprintf(stderr,"      (Output goes to standard output)(\n");
   fprintf(stderr,"\n");
   exit(1);
}


int main(int argc, char**argv)
  {
    int byte_counter;
    int byte;
#ifdef _WIN32
    setmode(fileno(stdin), O_BINARY);
    setmode(fileno(stdout), O_BINARY);
#endif


    /* Process LIF-filename from command line */
    if(argc != 1) {
       usage();
       exit(1);
    }
    byte_counter=0;
    while((byte=getchar())!=EOF) {
       if(byte_counter >= 32) putchar(byte);
       byte_counter++;
    }
   exit(0);
  }
