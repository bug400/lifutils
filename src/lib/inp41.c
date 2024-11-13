/* inp41.c -- A filter to decode an HP41 hexadecimal program */
/* 2000 A. R. Duell, and placed under the GPL */

/* This filter reads in an HP41 hexadecimal program (produced, for example
   by the Extended I/O command OUTP) and turns it into binary.
   
   Unlike a real HP41, this program ignores illegal characters, including
   whitespace, in the input.
 
   See outp41.c for details of the hexadecimal format */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"
#include "lifutils.h"

void inp41_usage(void)
  {
    fprintf(stderr,"Usage : lifutils inp41 < input device > output file\n");
    fprintf(stderr,"        Decode a HP41 hexadecimal program\n");
    fprintf(stderr,"\n");
  }


int inp41_read_nybble(void)
/* Read in characters until a valid hex digit and return its value */
  {
    int c,vld,nybble;

    do
      {
        vld=0;
        if((c=getchar())==EOF)
          {
            /* If EOF, give error and exit */
            fprintf(stderr,"inp41 : unexpected End Of File\n");
            return(RETURN_ERROR);
          }
        if((c>='0') && (c<='9'))
          {
            /* It's a digit */
            nybble=c-'0';
            vld=1;
          }
        if((c>='A') && (c<='F'))
          {
            /* It's an upper case hex letter */
            nybble=c-'A'+10;
            vld=1;
          }
        if((c>='a') && (c<='f'))
          {
            /* It's a lower case hex letter */
            nybble=c-'a'+10;
            vld=1;
          }
      } while(!vld);
    return(nybble);
  }

int inp41_read_byte(void)
  {
    int hi_nybble, lo_nybble;
    hi_nybble=inp41_read_nybble();
    return_if_error(hi_nybble);
    lo_nybble=inp41_read_nybble();
    return_if_error(lo_nybble);
    return((hi_nybble<<4)+lo_nybble);
  }

int inp41(int argc, char **argv)
  {
    int lo_length, hi_length, byte;
    int length, byte_counter;
    int checksum;

    int option;

    while ((option=getopt(argc,argv,"?"))!=-1)
      {
        switch(option)
          {
            case '?' : inp41_usage();
                       return(RETURN_OK);
          }
      }
    if(optind!=argc) {
       inp41_usage();
       return(RETURN_ERROR);
    }


    SETMODE_STDIN_BINARY;
    SETMODE_STDOUT_BINARY;
    checksum=0;
    hi_length=inp41_read_byte(); /* get high byte of length */
    return_if_error(hi_length);
    add_to_checksum(&checksum,hi_length);
    lo_length=inp41_read_byte(); /* get low byte of length */
    return_if_error(lo_length);
    add_to_checksum(&checksum,lo_length);
    /* calculate length */
    length=(hi_length<<8)+lo_length;

    /* Process program bytes */
    for(byte_counter=0; byte_counter<length; byte_counter++)
      {
        byte=inp41_read_byte(); 
        return_if_error(byte);
        add_to_checksum(&checksum,byte);
        putchar(byte);
      }

    /* Get checksum */
    byte=inp41_read_byte();
    return_if_error(byte);
    /* And check it */
    fprintf(stderr,"inp41 : checksum %s\n",(byte==checksum)?"good":"bad");
    return(RETURN_OK);
  }

