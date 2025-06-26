/* sdatabar.c -- convert an sdata file into HP41 barcode bytes */
/* 2001 A. R. Duell, and placed under the GPL */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "lifutils.h"
#include "config.h"
#include "lif_dir_utils.h"
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


void sdatabar_put_nybble(unsigned char *data, int nybble_counter, int nybble)
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


void sdatabar_put_digit(unsigned char *data, int nybble_counter, int digit, int row)
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
    sdatabar_put_nybble(data,nybble_counter,digit);
  }

void sdatabar_numeric_barcode(unsigned char *reg, int row, int kludge_flag)
/* Output an HP41 register reg[] as numeric barcode */
  {
    int checksum; /* Barcode row checksum */
    int nybble_counter; /* Counter for barcode nybbles */
    int exponent; /* Exponent of number in register */
    int i; /* loop counter */
    int no_decimal; /* The mantissa was output without a decimal point */
    unsigned char barcode[BARCODE_LENGTH]; /* buffer for output barcode */

    checksum=0;
    nybble_counter=0;
    no_decimal=0;

    /* clear barcode buffer */
    for(i=0; i<BARCODE_LENGTH; barcode[i++]=0);

    /* calculate exponent */
    exponent = ((reg[5]&0xf)*100) + ((reg[6]>>4)*10) + (reg[6]&0xf);
    exponent = (exponent>=500)?exponent-1000:exponent;

    sdatabar_put_nybble(barcode,nybble_counter++,6); /* type 6 barcode */

    /* Output the mantissa */
    if((reg[0]&0xf0)==0x90)
      {
        /* Mantissa is -ve */
        sdatabar_put_nybble(barcode,nybble_counter++,13); /* output '-' sign */
      }

    sdatabar_put_digit(barcode,nybble_counter++,reg[0]&0xf,row); /* output MS digit */

    if(kludge_flag && exponent && (exponent>=-90))
      {
        /* The number can be written without a decimal point */
        exponent-=9; /* correct the exponent */
        no_decimal=1;
      }
    else
      {
        sdatabar_put_nybble(barcode,nybble_counter++,11); /* output decimal point */
      }

    /* output next 8 digits */
    for(i=1; i<5; i++)
      {
        sdatabar_put_digit(barcode,nybble_counter++,reg[i]>>4,row);
        sdatabar_put_digit(barcode,nybble_counter++,reg[i]&0xf,row);
      }
    if(no_decimal)
      {
        /* If no decimal point, must output all mantissa digits */
        sdatabar_put_digit(barcode,nybble_counter++,reg[5]>>4,row); /* last digit */
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
                sdatabar_put_digit(barcode,nybble_counter++,reg[5]>>4,row); 
              }
          }
      }

    /* If the exponent is non-zero, output it */
    if(exponent)
      {
        sdatabar_put_nybble(barcode,nybble_counter++,14); /* output EEX */
        if(exponent<0)
          {
            if(exponent<-99)
              {
                /* Replace too small exponents with -99 */
                fprintf(stderr,"sdatabar : exponent too small,\n");
                fprintf(stderr,"           replaced by -99\n");       
                exponent=-99;
              }
            sdatabar_put_nybble(barcode,nybble_counter++,13); /* output '-' sign */
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
            sdatabar_put_digit(barcode,nybble_counter++,exponent/10,row);
          }
        sdatabar_put_digit(barcode,nybble_counter++,exponent%10,row);
      }

    /* If an odd number of nybbles output, put in a null */
    if(nybble_counter&1)
      {
        sdatabar_put_nybble(barcode,nybble_counter++,10);
      }

    /* calculate the checksum */
    for(i=0; i<(nybble_counter>>1); i++)
      {
        add_to_checksum(&checksum,barcode[i],TRUE);
      }

    /* output the barcode bytes */
    putchar(nybble_counter>>1); /* length of row - 1 */
    putchar(checksum); /* checksum */
    for(i=0; i<(nybble_counter>>1); i++)
      {
        putchar(barcode[i]); /* data bytes */
      }
  }

void sdatabar_alpha_barcode(unsigned char *reg,int row)
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
    add_to_checksum(&checksum,header_byte,TRUE);
    for(i=first_valid; i<7; i++)
      {
        add_to_checksum(&checksum,reg[i],TRUE);
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

void sdatabar_usage(void)
  {
    fprintf(stderr,"Usage : lifutils sdatabar [-e][-r] INPUTFILE > output file\n");
    fprintf(stderr,"        or\n");
    fprintf(stderr,"        lifutils sdatabar [-e][-r] < input file > output file\n");
    fprintf(stderr,"        Convert a raw SDATA file to an intermediate barcode file\n");
    fprintf(stderr,"        -e : Correct 1E ROM HP41 wand bug\n");
    fprintf(stderr,"        -r skip an existing LIF header of the input file\n");
    fprintf(stderr,"\n");
  }

int sdatabar(int argc, char **argv)
  {
    unsigned char record[SDATA_RECORD_SIZE]; /* file record */
    unsigned char reg[REGISTER_SIZE]; /* corresponding HP41 register */
    int row; /* row of barcode */
    int kludge_flag; /* Attempt to get round the bug in 1E wands? */
    FILE *fp;
    int skip_lif=0;
    int option; /* Command line option character */


    row=1;
    kludge_flag=0;
    optind=1;

    /* Decode command line options */
    while((option=getopt(argc,argv,"er?"))!=-1)
      {
        switch(option)
          {
            case 'e' : kludge_flag=1; /* Set kludge_flag for 1E wand */
                       break;
            case 'r' : skip_lif=1;
                       break;
            case '?' : sdatabar_usage();
                       return(RETURN_OK);
          }
      }
     if((optind!=argc) && (optind!= argc-1))

       {
         sdatabar_usage();
         return(RETURN_ERROR);
       }
     if(optind== argc-1) {
        fp=fopen(argv[argc-1],"rb");
     } else {
        fp=stdin;
        SETMODE_STDIN_BINARY;
     }

     if (fp == NULL) {
       fprintf(stderr,"Error: cannot open input file\n");
       return(RETURN_ERROR);
     }

    if(skip_lif) {
       if(skip_lif_header(fp,"SDATA")) {
          fclose(fp);
          fprintf(stderr,"Error: not a LIF HP-41 sdata file\n");
          return(RETURN_ERROR);
       } 
    } 

    /* read in file and convert it */
    SETMODE_STDOUT_BINARY;
    while(fread(record,sizeof(unsigned char),SDATA_RECORD_SIZE,fp)==SDATA_RECORD_SIZE) 
      {
        descramble(record,reg); /* descramble the nybbles */
        switch(reg[0]>>4) /* decode the type nybble */
          {
            case 0 :
            case 9 : /* numeric record */
                     sdatabar_numeric_barcode(reg,row++,kludge_flag);
                     break;
            case 1 : /* alpha record */
                     sdatabar_alpha_barcode(reg,row++); 
                     break;
            default : /* unknown */
                     fprintf(stderr,"sdatabar : unknown register contents\n");
                     fprintf(stderr,"           ... skipping\n");
                     break;
          }
      }
    if(fp!=stdin)fclose(fp);
    return(RETURN_OK);
  }
