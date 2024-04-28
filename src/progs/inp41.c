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

unsigned char read_nybble(void)
/* Read in characters until a valid hex digit and return its value */
  {
    int c;
    unsigned char vld,nybble;

    do
      {
        vld=0;
        if((c=getchar())==EOF)
          {
            /* If EOF, give error and exit */
            fprintf(stderr,"inp41 : unexpected End Of File\n");
            exit(1);
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

unsigned char read_byte(void)
  {
    unsigned char hi_nybble, lo_nybble;
    hi_nybble=read_nybble();
    lo_nybble=read_nybble();
    return((hi_nybble<<4)+lo_nybble);
  }

void add_to_checksum(unsigned int *checksum, unsigned char byte)
/* Modulo 256 addition to checksum */
  {
    *checksum+=byte;
    *checksum&=0xff;
  }

int main(int argc, char **argv)
  {
    unsigned char lo_length, hi_length, byte;
    unsigned int length, checksum, byte_counter;

    SETMODE_STDIN_BINARY;
    SETMODE_STDOUT_BINARY;
    checksum=0;
    hi_length=read_byte(); /* get high byte of length */
    add_to_checksum(&checksum,hi_length);
    lo_length=read_byte(); /* get low byte of length */
    add_to_checksum(&checksum,lo_length);
    /* calculate length */
    length=(hi_length<<8)+lo_length;

    /* Process program bytes */
    for(byte_counter=0; byte_counter<length; byte_counter++)
      {
        byte=read_byte(); 
        add_to_checksum(&checksum,byte);
        putchar(byte);
      }

    /* Get checksum */
    byte=read_byte();
    /* And check it */
    fprintf(stderr,"inp41 : checksum %s\n",(byte==checksum)?"good":"bad");
    exit(0);
  }

