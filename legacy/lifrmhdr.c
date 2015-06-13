/* lifrmhdr.c -- A filter to remove 'anadisk-style' headers from a LIF disk 
   image */ 
/* 2001 A. R. Duell, and placed under the GPL */

/* See lifaddhdr.c for details of the disk header format. Since anadisk 
   does not write sectors in increasing numerical order (in general), it
   is necessary to slurp the entire disk image into core and then dump it
   out again. */

#include<stdio.h>

/* 'Dimensions' of a LIF disk */
#define CYLINDERS 77
#define HEADS 2
#define SECTORS 16
#define SECTOR_SIZE 256

/* Bytes in the Anadisk header */
#define ACT_CYL 0 
#define ACT_HD 1
#define LOG_CYL 2
#define LOG_HD 3
#define SEC 4
#define LEN_CODE 5
#define LEN_LO 6
#define LEN_HI 7
#define HDR_LEN 8 /* size of the header */

/* An array to hold the disk image */
unsigned char data[CYLINDERS][HEADS][SECTORS][SECTOR_SIZE];

/* And one to hold a flag to indicate that each sector has been read */
unsigned char flags[CYLINDERS][HEADS][SECTORS];

void clear_flags(void)
  {
    /* Clear all the flags to indicate that no sectors have been read */
    int cylinder,head,sector;
    for(cylinder=0; cylinder<CYLINDERS; cylinder++)
      {
        for(head=0; head<HEADS; head++)
          {
            for(sector=0; sector<SECTORS; sector++)
              {
                flags[cylinder][head][sector]=0;
              }
          }
      }
  }

void read_image(void)
  {
    /* Read in the disk image, interpretting the headers */

    unsigned char header[HDR_LEN]; /* header record */
    unsigned char dummy[65536]; /* Store for bad sectors */

    while(fread(header,sizeof(char),HDR_LEN,stdin)==HDR_LEN)
      {
        /* Check Header */
        if((header[ACT_CYL]!=header[LOG_CYL]) /* Logical and physical 
                                                 cylinders different */
          ||(header[ACT_HD]!=header[LOG_HD]) /* ditto, for heads */
          ||(header[LOG_CYL]>=CYLINDERS)     /* Cylinder too large */
          ||(header[LOG_HD]>=HEADS)          /* Head too large */
          ||(header[SEC]<1)                  /* There is no sector 0 */
          ||(header[SEC]>SECTORS)            /* Sector # too large */
          ||(header[LEN_LO]!=0)              /* Low byte of length */
          ||(header[LEN_HI]!=1))             /* High byte of length */ 
          {
            fprintf(stderr,"Illegal header ! \n");
            fprintf(stderr,"Actual cylinder = %d, Actual head = %d\n",
                    header[ACT_CYL],header[ACT_HD]);
            fprintf(stderr,"Logical cylinder = %d, Logical head = %d\n",
                    header[LOG_CYL],header[LOG_HD]);
            fprintf(stderr,"Sector = %d\n",header[SEC]);
            fprintf(stderr,"Length = %d\n",header[LEN_LO]+(header[LEN_HI]<<8));
            fread(dummy,sizeof(char),header[LEN_LO]+(header[LEN_HI]<<8),stdin);
          }
        else
          {
            /* OK, the header looks good !, read in the data */
            if(fread(data[header[ACT_CYL]][header[ACT_HD]][header[SEC]-1],
                     sizeof(char),SECTOR_SIZE,stdin)!=SECTOR_SIZE)
              { 
                fprintf(stderr,"unexpected end of input while reading data\n");
                exit(1);
              }
            /* Read in the sector, set the flag to say it's OK */
            flags[header[ACT_CYL]][header[ACT_HD]][header[SEC]-1]=1;
          }
      }
  }

void check_flags(void)
  {
    /* Check to ensure all sectors have been read */
    int cylinder,head,sector;
    for(cylinder=0; cylinder<CYLINDERS; cylinder++)
      {
        for(head=0; head<HEADS; head++)
          {
            for(sector=0; sector<SECTORS; sector++)
              {
                if(!flags[cylinder][head][sector])
                  {
                    fprintf(stderr,
                    "Missing sector : Cylinder = %d, Head = %d, Sector = %d\n"
                            ,cylinder,head,sector+1);
                  }
              }
          }
      }
  }
  
void dump_image(void)
  {
    /* Now dump the image to stdout */
    int cylinder,head,sector;
    for(cylinder=0; cylinder<CYLINDERS; cylinder++)
      {
        for(head=0; head<HEADS; head++)
          {
            for(sector=0; sector<SECTORS; sector++)
              {
                fwrite(data[cylinder][head][sector],sizeof(char),
                       SECTOR_SIZE,stdout);
              }
          }
      }
  }

int main(int argc, char **argv)
  {
     clear_flags();
     read_image();
     check_flags();
     dump_image();
  }
