/* er41rom.c -- unscramble an HP41 ROM dump file */
/* renamed from the previous rom41.c program */
/* 2001 A. R. Duell, and placed under the GPL */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"

/* The Eramco MLDL-OS ROM can save HP41 ROM (or MLDL RAM bank) images in 
a ROM41 file. 4 (10 bit) HP41 words are stored in 5 bytes of the file : 

Byte 0 : High bits (2 bits for each word, MSB for word 3)
Byte 1 : Low byte, word 3
Byte 2 : Low byte, word 2
Byte 3 : Low byte, word 1
Byte 4 : Low byte, word 0

This program unscrambles that data, and outputs 2 bytes for each HP41 
word. The first byte contains the high 2 bits, the second byte contains 
the low 8 bits */

/* Number of bytes in an input file record */
#define RECORD_SIZE 5 

int main(int argc, char **argv)
  {
    unsigned char scrambled_data[RECORD_SIZE]; /* Input data words */

    SETMODE_STDIN_BINARY;
    SETMODE_STDOUT_BINARY;

    /* read in the file data, 1 record at a time */
    while(fread(scrambled_data,sizeof(unsigned char),RECORD_SIZE,stdin)==
          RECORD_SIZE)
      {
       /* output the unscrambled data */
        putchar(scrambled_data[0]&3); /* MSB word 0 */
        putchar(scrambled_data[4]); /* LSB word 0 */
        putchar((scrambled_data[0]>>2)&3); /* MSB word 1 */
        putchar(scrambled_data[3]); /* LSB word 1 */
        putchar((scrambled_data[0]>>4)&3); /* MSB word 2 */
        putchar(scrambled_data[2]); /* LSB word 2 */
        putchar((scrambled_data[0]>>6)&3); /* MSB word 3 */
        putchar(scrambled_data[1]); /* LSB word 3 */
      }
    exit(0);
  }
