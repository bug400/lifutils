/* sdatabar.c -- convert an sdata file into HP41 barcode bytes */
/* 2001 A. R. Duell, and placed under the GPL */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"
#include"descramble_41.h"

/* This program reads in an sdata file from stadnard input and converts 
   numeric 'records' into numeric data (type 6) barcode and alpha records 
   into alpha replace (type 7) barcode. The barcode bytes are written to 
   standard output, and can be piped to a suitable printer driver. 

   The format of HP41 barcode is given in 'Create your own HP41 Barcode'
   (Hewlett Packard) and, in a more abreviated form, in the 'Synthetic
   Programming Quick Reference Guide' (Jeremy Smith) 

   One problem is that version 1E wands have a number of bugs in the ROM
   code. In particular, they can't read numeric data barcode containing
   10 or more bytes. Unfortunately, a number consisting of a -ve 10 digit
   mantissa and -ve 2 digit exponent may require : 

   10 nybbles : mantissa digits
   2  nybbles : exponent digits
   2  nybbles : signs
   1  nybble  : decimal point
   1  nybble  : 'EEX marker'
   1  nybble  : barcode type indicator (set to 6)
   --
   17 nybbles total
   Add one nybble to make the count even, and there are 18 nybbles, or 9 
   bytes. Add one byte for the checksum, and it's too long.

   If the parameter kludge_flag to the numeric_barcode routine is 
   non-zero, numeric_barcode attempts to remedy this if it by omitting the 
   decimal point and decreasing the exponent to compensate. This is not 
   possible if the expoent is <-90, in which case a suitable message is 
   written to standard error and the last mantissa digit is omitted from
   the barcode. */


/* size of HP41 register and sdata file record in bytes */
#define REGISTER_SIZE 7
#define RECORD_SIZE 8
/* maximum length of a barcode row */
#define BARCODE_LEN 16

void add_to_checksum(int *checksum, unsigned char data)
/* Add data to checksum using the end-around-carry algorithm */
  {
    (*checksum) += data; /* add the new data */
    if((*checksum)>255)
      {
        /* There is a carry */
        (*checksum)++; /* add it end-around */
      }
    (*checksum) %= 256; /* keep only the low byte */
  }

void put_nybble(unsigned char *data, int nybble_counter, int nybble)
/* Put a nybble into the appropriate position in data[] */
  {
    nybble &= 0xf; /* Ensure only the appropriate bits are changed */
    if(nybble_counter&1)
      {
        /* Odd numbered nybble -- low 4 bits */
        data[nybble_counter>>1] &=0xf0; /* leave high bits alone */
        data[nybble_counter>>1] |= nybble; /* stick in new nybble */
      }
    else
      {
        /* Even numbered nybble -- high 4 bits */
        data[nybble_counter>>1] &= 0xf; /* leave low bits alone */
        data[nybble_counter>>1] |= (nybble << 4); /* stick in new nybble */
      }
  }


void put_digit(unsigned char *data, int nybble_counter, int digit, int row)
/* Put in a digit, give error (and replace it by 9) if illegal */
  {
    digit &= 0xf; /* Mask out unused bits */
    if(digit>9)
      {
        /* This is an non-normalised number */
        fprintf(stderr,"sdatabar : Illegal digit, possible non-normalised\n");
        fprintf(stderr,"           number in row %d. Replaced by 9\n",row);
        digit=9; /* Replace illegal digit with 9 */
      }
    put_nybble(data,nybble_counter,digit);
  }

void numeric_barcode(unsigned char *reg, int row, int kludge_flag)
/* Output an HP41 register reg[] as numeric barcode */
  {
    int checksum; /* Barcode row checksum */
    int nybble_counter; /* Counter for barcode nybbles */
    int exponent; /* Exponent of number in register */
    int i; /* loop counter */
    int no_decimal; /* The mantissa was output without a decimal point */
    unsigned char barcode[BARCODE_LEN]; /* buffer for output barcode */

    checksum=0;
    nybble_counter=0;
    no_decimal=0;

    /* clear barcode buffer */
    for(i=0; i<BARCODE_LEN; barcode[i++]=0);

    /* calculate exponent */
    exponent = ((reg[5]&0xf)*100) + ((reg[6]>>4)*10) + (reg[6]&0xf);
    exponent = (exponent>=500)?exponent-1000:exponent;

    put_nybble(barcode,nybble_counter++,6); /* type 6 barcode */

    /* Output the mantissa */
    if((reg[0]&0xf0)==0x90)
      {
        /* Mantissa is -ve */
        put_nybble(barcode,nybble_counter++,13); /* output '-' sign */
      }

    put_digit(barcode,nybble_counter++,reg[0]&0xf,row); /* output MS digit */

    if(kludge_flag && exponent && (exponent>=-90))
      {
        /* The number can be written without a decimal point */
        exponent-=9; /* correct the exponent */
        no_decimal=1;
      }
    else
      {
        put_nybble(barcode,nybble_counter++,11); /* output decimal point */
      }

    /* output next 8 digits */
    for(i=1; i<5; i++)
      {
        put_digit(barcode,nybble_counter++,reg[i]>>4,row);
        put_digit(barcode,nybble_counter++,reg[i]&0xf,row);
      }
    if(no_decimal)
      {
        /* If no decimal point, must output all mantissa digits */
        put_digit(barcode,nybble_counter++,reg[5]>>4,row); /* last digit */
      }
    else
      {
        /* There was a decimal point */
        if(reg[5]>>4)
          { 
            /* Last mantissa digit is non-zero */
            if(kludge_flag && exponent && (exponent<-9) && (reg[0]&0xf0)==0x90)
              {
                /* There are too many characters */
                fprintf(stderr,
                        "sdatabar : dropping last mantissa digit in row %d\n",
                        row);
               }
            else
              {
                /* Output the digit */
                put_digit(barcode,nybble_counter++,reg[5]>>4,row); 
              }
          }
      }

    /* If the exponent is non-zero, output it */
    if(exponent)
      {
        put_nybble(barcode,nybble_counter++,14); /* output EEX */
        if(exponent<0)
          {
            if(exponent<-99)
              {
                /* Replace too small exponents with -99 */
                fprintf(stderr,"sdatabar : exponent too small,\n");
                fprintf(stderr,"           replaced by -99\n");       
                exponent=-99;
              }
            put_nybble(barcode,nybble_counter++,13); /* output '-' sign */
            exponent *= -1; /* make exponent +ve */
          }
        else
          {
            if(exponent>99)
              {
                /* replace too large exponents with 99 */
                fprintf(stderr,"sdatabar : exponent too large,\n");
                fprintf(stderr,"           replaced by 99\n");       
                exponent=99;
              }
          }
        /* output exponent digits */
        if(exponent/10)
          {
            put_digit(barcode,nybble_counter++,exponent/10,row);
          }
        put_digit(barcode,nybble_counter++,exponent%10,row);
      }

    /* If an odd number of nybbles output, put in a null */
    if(nybble_counter&1)
      {
        put_nybble(barcode,nybble_counter++,10);
      }

    /* calculate the checksum */
    for(i=0; i<(nybble_counter>>1); i++)
      {
        add_to_checksum(&checksum,barcode[i]);
      }

    /* output the barcode bytes */
    putchar(nybble_counter>>1); /* length of row - 1 */
    putchar(checksum); /* checksum */
    for(i=0; i<(nybble_counter>>1); i++)
      {
        putchar(barcode[i]); /* data bytes */
      }
  }

void alpha_barcode(unsigned char *reg,int row)
/* Output an HP41 register as alpha-replace barcode */
  {
    int checksum; /* barcode checksum */
    unsigned char header_byte; /* barcode type/#chars */
    int first_valid; /* first non-null character in register */
    int i; /* loop counter */;

    checksum=0;

    if(reg[0]&0xf)
      {
        /* bits set in zeroth low nybble */
        fprintf(stderr,"sdatabar  : bldspec high bit lost in row %d\n",row);
      }

    /* Find first non-null character */
    /* first_valid will be less than or equal to 6, so at least one
       character is always output */
    for(first_valid=1; first_valid<6; first_valid++)
      {
        if(reg[first_valid])
          {
            /* This character is not null */
            break;
          }
      }

    header_byte=0x70+(7-first_valid); /* barcode type and length */

    /* calculate checksum */    
    add_to_checksum(&checksum,header_byte);
    for(i=first_valid; i<7; i++)
      {
        add_to_checksum(&checksum,reg[i]);
      }

    /* output barcode */
    putchar(8-first_valid); /* barcode row length */
    putchar(checksum);
    putchar(header_byte);
    for(i=first_valid; i<7 ; i++)
      {
        putchar(reg[i]);
      }
  }

void usage(void)
  {
    fprintf(stderr,"Usage : sdatabar [-e]\n");
    exit(1);
  }

int main(int argc, char **argv)
  {
    unsigned char record[RECORD_SIZE]; /* file record */
    unsigned char reg[REGISTER_SIZE]; /* corresponding HP41 register */
    int row; /* row of barcode */
    int kludge_flag; /* Attempt to get round the bug in 1E wands? */
    int option; /* Command line option character */

    SETMODE_STDIN_BINARY;
    SETMODE_STDOUT_BINARY;

    row=1;
    kludge_flag=0;
    optind=1;

    /* Decode command line options */
    while((option=getopt(argc,argv,"e?"))!=-1)
      {
        switch(option)
          {
            case 'e' : kludge_flag=1; /* Set kludge_flag for 1E wand */
                       break;
            case '?' : usage();
          }
      }

    /* read in file and convert it */
    while(fread(record,sizeof(unsigned char),RECORD_SIZE,stdin)==RECORD_SIZE) 
      {
        descramble(record,reg); /* descramble the nybbles */
        switch(reg[0]>>4) /* decode the type nybble */
          {
            case 0 :
            case 9 : /* numeric record */
                     numeric_barcode(reg,row++,kludge_flag);
                     break;
            case 1 : /* alpha record */
                     alpha_barcode(reg,row++); 
                     break;
            default : /* unknown */
                     fprintf(stderr,"sdatabar : unknown register contents\n");
                     fprintf(stderr,"           ... skipping\n");
                     break;
          }
      }
    exit(0);
  }
