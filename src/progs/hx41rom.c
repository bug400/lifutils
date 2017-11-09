/* hx41rom.c -- a filter to descramble HEPAX rom files */
/* 2015 Joachim Siebold and placed under the GPL */

/*  reads a HP-41 data file in hepax format (without header) an descrambles it to an unpacked rom file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include "config.h"
#include "lif_const.h"


#define HEPAX_RECORD_SIZE 5 
#define ROM_RECORD_SIZE 8

#define TRUE 1
#define DEBUG 0
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

void usage(void)
{
   fprintf(stderr,
   "Usage: hx41rom \n");
   fprintf(stderr,"      (Input comes from standard input)\n");
   fprintf(stderr,"      (Output goes to standard output)(\n");
   fprintf(stderr,"\n");
   fprintf(stderr,"\n");
   exit(1);
}

int main(int argc, char**argv)
  {

    unsigned char hepax_data[HEPAX_RECORD_SIZE]; /* Input data bytes */
    unsigned char rom_data[ROM_RECORD_SIZE]; /* Output data bytes */
    int i;

    SETMODE_STDIN_BINARY;
    SETMODE_STDOUT_BINARY;


    /* Process command line */
    if(argc != 1) {
       usage();
       exit(1);
    }

    /* read in the file data, 1 record at a time */
    while(fread(hepax_data,sizeof(unsigned char),HEPAX_RECORD_SIZE,stdin)==
          HEPAX_RECORD_SIZE) {
          /* unscramble data */
         rom_data[0]= hepax_data[0] >> 6;
         rom_data[1]= ((hepax_data[0] & 0x3F) << 2) | (hepax_data[1] >> 6); 
         rom_data[2]= (hepax_data[1] & 0x30) >> 4;
         rom_data[3]= ((hepax_data[1] & 0xF) << 4 ) | ((hepax_data[2] & 0xF0) >> 4);
         rom_data[4]= (hepax_data[2] & 0xC) >> 2;
         rom_data[5]= ((hepax_data[2] & 0x3) << 6) | ((hepax_data[3] & 0xFC) >> 2);
         rom_data[6]= (hepax_data[3] & 0x3);
         rom_data[7]= hepax_data[4];
         for(i=0; i< ROM_RECORD_SIZE ; i++) 
            putchar((int) rom_data[i]);
    }
    exit(0);
  }
