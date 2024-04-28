/* regs41.c -- a filter to display descrambled registers from an HP41 file */ 
/* 2000 A. R. Duell, and placed under the GPL */

/* This is primarily a debugging tool used to analyse 'unknown' HP41
   files. It displays the raw HP41 register data from that file */

#include<stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"
#include"descramble_41.h"

/* Length of a file record */
#define RECORD_LEN 8

/* length of an HP41 register */
#define REGISTER_LEN 7

void print_register(unsigned char *reg)
  {
    /* print the data in one register in hex, followed by newline */
    int i; /* byte counter */
    for(i=0; i<REGISTER_LEN; i++)
      {
        printf(" %02x",reg[i]);
      }
    printf("\n");
  }

int main(int argc, char **argv)
  {
    unsigned char record[RECORD_LEN]; /* file record */
    unsigned char reg[REGISTER_LEN]; /* corresponding HP41 register */
    int reg_number=0; /* register counter */

    SETMODE_STDIN_BINARY;
 
    /* read in records, translate, print */
    while(fread(record,sizeof(char),RECORD_LEN,stdin)==RECORD_LEN)
      {
        descramble(record,reg);
        printf("%04x :",reg_number);
        print_register(reg);
        reg_number++;
      }
    exit(0);
  }
