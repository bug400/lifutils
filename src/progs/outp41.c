/* outp41.c -- A filter to produce a hexadecimal version of an HP41 
   program, similar to that used by the Extended I/O commands OUTP and 
   INP */
/* 2000 A. R. Duell, and placed under the GPL */

/* The Extended I/O module has 2 commands INP and OUTP which allow HP41
   user-code programs to be transferred between machines as printable
   hexadecimal files. OUTP reads a (non-private) program from the HP41's
   memory and sends it in hex to the default I/O device. INP does the 
   reverse; it reads a hex stream from the default I/O device and stores 
   it as an HP41 program in the machine's memory.

   The format of the hexadecimal stream appears not to be documented in any
   of the manuals, so here it is : 

   Each byte is sent as 2 characters (0..9,A..F) in the obvious way (high
   nybble first). No other characters (not even whitespace) appear in the 
   stream.

   The data sent is as follows : 

   Program length (2 bytes/4 characters), high byte first
   Program data (<length> bytes, 2*<length> characters) in the obvious order
   Checksum (1 byte/2 characters). This is the modulo-256 sum of all 
   bytes (not characters!) that have come before, including the lentgh.

   */

#include<stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"

/* MEMORY_SIZE is larger than a real HP41's memory, so that any normal
   program can be loaded */

#define MEMORY_SIZE 4096

int read_prog(unsigned char *memory)
/* Read an HP41 program in from standard input */
  {
    int byte_counter;
    int byte;

    byte_counter=0; /* Start loading at the start of 'memory' */
    while(((byte=getchar())!=EOF) && (byte_counter<MEMORY_SIZE))
      {
        memory[byte_counter]=byte;
        byte_counter++;
      }
    return(byte_counter); /* Number of bytes read */
  }


void add_to_checksum(unsigned int *checksum, unsigned char byte)
/* Modulo 256 addition of byte to checksum */
  {
    *checksum+=byte;
    *checksum&=0xFF;
  }

void print_hex(unsigned char byte)
/* Print byte in hex to standard output */
  {
    printf("%02X",byte);
  }

int main (int argc, char **argv)
  {
    unsigned int checksum=0;
    int length;
    int byte_counter;
    unsigned char memory[MEMORY_SIZE];

    SETMODE_STDIN_BINARY;
    SETMODE_STDOUT_BINARY;

    length=read_prog(memory);
    print_hex(length>>8); /* Send high byte of length */
    add_to_checksum(&checksum,length>>8);
    print_hex(length&0xff); /* Send low byte of length */
    add_to_checksum(&checksum,length&0xff);

    /* Send the program bytes */
    for(byte_counter=0; byte_counter<length; byte_counter++)
      {
        print_hex(memory[byte_counter]);
        add_to_checksum(&checksum,memory[byte_counter]);
      }

    print_hex(checksum); /* send checksum */
    exit(0);
  }

