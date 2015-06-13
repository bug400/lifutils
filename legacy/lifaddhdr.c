/* lifaddhdr.c -- A filter to add 'anadisk-style' headers to a LIF disk 
   image */
/* 2000 A. R. Duell, and placed under the GPL */

/* Anadisk (a shareware disk analysing program for MS-DOS) can write a disk
   image file where the data from each sector is preceded by an 8 byte 
   header. This header contains the following values : 
   Actual Cylinder (1 byte)
   Actual Head (1 byte)
   Logical Cylinder (1 byte)
   Logical Head (1 byte)
   Sector (1 byte)
   Length code (as definded in the 8272 data sheet) (1 byte)
   Actual number of data bytes (2 bytes).

   For a LIF disk, the logical and physical cylinder numbers are the 
   same, as are the logical and physical head numbers. Each sector contains
   256 bytes and has a length code of 1. This simplifies things. */

#include <stdio.h>

#define SECTOR_SIZE 256

int main(int argc, char **argv)
  {
    unsigned int cylinder,head,sector;
    unsigned char *data[SECTOR_SIZE]; /* Buffer for one sector of data */

    /* Read in each sector from stdin, write the header and the sector
       to stdout */
    for(cylinder=0; cylinder<77; cylinder++)
      {
        for(head=0; head<2; head++)
          {
            for(sector=1; sector<17; sector++)
              {
                /* Read in a sector */
                if(fread(data,sizeof(char),SECTOR_SIZE,stdin)!=SECTOR_SIZE)
                  {
                    fprintf(stderr,"lifaddhdr: Unexpected end of input\n");
                    exit(1);
                  }
                /* Output the header */
                putchar(cylinder); /* Physical cylinder */
                putchar(head); /* Physical head */
                putchar(cylinder); /* Logical cylinder */
                putchar(head); /* Logical head */
                putchar(sector); /* Sector number */
                putchar(1); /* Length code */
                putchar(0); putchar(1); /* 256 bytes/sector */
                /* Output the data */
                fwrite(data,sizeof(char),SECTOR_SIZE,stdout);
              }
          }
      }
    exit(0);
  }    
