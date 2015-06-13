/* barprt.c -- a diagnostic printing filter for barcode files */
/* 2001 A. R. Duell, and placed under the GPL */

#include<stdio.h>
#include <stdlib.h>
#include <fcntl.h>

/* This filter reads a barcode file from standard input and prints an 
   (hopefully) human-readable form of it on standard output. It is 
   intended as a diagnostic tool for barcode generation and printing 
   programs */

/* maximum length of a row of barcode in bytes */
#define BARCODE_LENGTH 16

int main(int argc, char **argv)
  {
    int row_length; /* length of this barcode row in bytes */
    unsigned char barcode[BARCODE_LENGTH]; /* one row of barcode */
    int i; /* loop counter */
    int row; /* current row number */

#ifdef _WIN32
    setmode(fileno(stdin), O_BINARY);
#endif
    row=0;
    while((row_length=getchar())!=EOF)
      {
        row_length &=0xf; /* Row length is LS nybble + 1 */
        row_length++;
        if(fread(barcode,sizeof(unsigned char),row_length,stdin)!=row_length)
          {
            fprintf(stderr,"Unexpected end of file reading barcode\n");
            exit(1);
          }
        row++; /* increment barcode row number */
        printf("Row : %2d , %2d bytes\n", row, row_length);
        printf("HEX :");
        for(i=0; i<row_length; i++)
          {
            printf("  %02x",barcode[i]);
          }
        printf("\nDEC :");
        for(i=0; i<row_length; i++)
          {
            printf(" %3d",barcode[i]);
          }
        printf("\n");
      }
    exit(0);
  }

