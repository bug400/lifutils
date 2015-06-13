/* print_41_data.c -- routines to print the contents of HP41 (and SDATA) 
    files, etc */
/* 2000 A. R. Duell and placed under the GPL */

/* Various HP41 files and SDATA files (used on the HP41 and HP71)  
    consist of 8-byte records. Each record can represent : 

   a) A real number. If the low nybble of the second byte is 0, then
      the record is a +ve number, while if it's 9, the record is a -ve number
      The first 3 nybbles represent the 3-digit exponent, in the order 
      middle, low, high (!). The last 6 bytes of the record store the 
      mantissa, with the last (eighth) byte containing the most significant
      digits, and the remainder in reverse byte order from there. All 
      digits are stored in BCD.

      Exponent values 0--499 represent +ve exponents 0--499, while values 
      501-999 represent -ve exponents -499 -- -1
      If the high nybble of the exponent is 0xF, then the number is either
      a NaN (if the rest of the exponent is non-zero) or infinity (if the 
      rest of the exponent is 0), whereupon the sign nybble gives the sign
      of infinity.

   b) A text string. The low nybble of the second byte is 1. Six
      Ascii characters are packed in as follows : 

      6H 6L | 5L  1 | 0 0 | 4L 5H | 3L 4H | 2L 3H | 1L 2H | X 1H 

     The nybble X is normally 0, but contains the extra bit
     used to pack the 49 bits of a BLDSPEC character into the 
     record (there are 48 bits in the 6 characters). If X and 1H are
     both 0xF, then the string was produced by RCLFLAG and the remaining 11 
     character nybbles contain the settings of flags 0-43 

     Further information can be found in the HP71 Software IDS 
     Volume 1 Section 11.2.4.2 and 11.2.7 (although the string
     format is misdocumented there). Also see the routines 
     RED41C (0x13F28) 
     N-STR  (0x14BFC)
     [Addresses given are for the 1BBBB ROM]
     in volume 3 of the HP71 Software IDS */

#include<stdio.h>
#include<ctype.h>
#include <stdlib.h>
#include"descramble_41.h"

#define RECORD_LENGTH 8

void print_numeric(unsigned char *data, int length_flag)
  {
    /* Print a record that contains a number */
    int i; /* digit counter */
    int exponent;
    if((data[1]>>4)==0xF)
      {
        /* This is a NaN or Infinity */
        if(data[0])
          {
            printf("NaN\n");
          }
        else
          {
            printf("%cinf\n",((data[1]&0xF)==9)?'-':'+');
          }
      } 
    else
      {
        /* OK, it's a normal number */
        /* print the sign and first 2 digits of the mantissa */
        printf("%c%1x.%1x",((data[1]&0xF)==9)?'-':'+',
               data[7]>>4, data[7]&0xF);
        /* print the rest of the mantissa */
        for(i=6; i>(length_flag?1:2); i--)
           {
             /* This is a fiddle to print 2 BCD digits at once */
             printf("%02x",data[i]);
           }
        /* decode the exponent */
        exponent=100*(data[1]>>4) + 10*(data[0]>>4) + (data[0]&0xF);
        exponent=(exponent>500)?exponent-1000:exponent;
        /* print the exponent */
        printf("E%+d\n",exponent);
      }
  }

void print_char(unsigned char c)
  {
    /* print a character, 'normally' if printable, otherwise as a \nnn
       octal escape sequence */
    if(isprint(c))
     {
       putchar(c);
     }
   else
     {
       printf("\\%03o",c);
     }
  }
 
void print_string(unsigned char *str)
  {
    /* Print a 6-character string, skipping leading nulls */
    int skipping; /* flag for skipping over nulls */
    int i; /* Character counter */

    skipping=1; /* Initially, skip over nulls */
    printf("\""); /* Print double quote */
    for (i=1; i<7; i++)
      {
        skipping=str[i]?0:skipping; /* Clear skipping if non-null character */
        if(!skipping)
          {
            print_char(str[i]);
          }
      }
    printf("\"\n");
  }

void print_bldspec(unsigned char *str)
  {
    /* The HP41 printer command BLDSPEC packs a 7*7 character dot matrix 
       definition into a 6 byte string, with one bit in the otherwise unused
       nybble. */
    int i;/* character counter */
    for(i=1; i<7; i++)
      {
        /* Extract the appropriate 7 bits from the string and print them */
        printf("%3d; ",((str[i-1]<<(7-i))&0x7f)+(str[i]>>(i+1)));
      }
    printf("%3d; \n",str[6]&0x7f); /* Print last character */
  }

void print_binary(unsigned char c, int length)
  {
     /* Print length bits of c in binary, MSB first */
     int i; /* bit counter */
     c=c<<(8-length); /* get wanted bits to the most significant end */
     for(i=0; i<length; i++)
       {
         printf("%1d",(c&0x80)?1:0); /* Print a bit */
         c=c<<1; /* shift for next bit */
       }
  }

void print_flags(unsigned char *str)
  {
    /* Print 44 bits corresponding to flags 0-43. It is known (Synthetic
       Quick Reference Guide) that flag 0 is the MSB of the flag register
       (Status Register d). Assume that it is also the MSB when the data
       is moved into the string */

    int i; /* Byte counter */
    printf("FLAGS : ");
    for(i=1; i<7; i++)
      {
        /*Print the flags in binary. The first byte only contains
          4 flags, the others contain 8 */
        print_binary(str[i],(i==1)?4:8);
      }
    printf("\n");
  }

void print_alpha(unsigned char *data, int bldspec_flag, int flags_flag)
  {
    unsigned char str[7]; /* decoded string */
    descramble(data,str);
    if(bldspec_flag)
      {
        print_bldspec(str);
      }
    else
      {
        if(flags_flag && (data[7]==0xff))
          {
            print_flags(str);
          }
        else
          {
            print_string(str);
          }
      }
  }

void print_hex(unsigned char *data)
  {
    /* Print a record in hex */
    int i; /* counter */
    for(i=0; i<RECORD_LENGTH; i++)
      {
        printf("%02x ",data[i]);
      }
    printf("\n");
  }


void print_record(unsigned char *record, int length, int bldspec_flag, 
                  int flags_flag) 
  {
    /* print a signle 8-byte record from an SDATA (or similar) file */
    int sign; /* Sign nybble from the record */
    sign=record[1]&0xf;
    switch(sign)
      {
        case 0:
        case 9: print_numeric(record,length);
                break;
        case 1: print_alpha(record,bldspec_flag,flags_flag);
                break;
        default: printf("Unknown data format\n");
                 break;
      }
  }
