/* lifimage.c -- make an image file of a physical LIF disk */
/* 2000 A. R. Duell, and placed under the GPL */

/* This program creates an image file of a physical HP LIF disk. This image
   file consists of the data from : 
   (Cy 0, H 0, S 1)...(Cy 0, H 0, S 16)(Cy 0, H 1, S 1)...(Cy 0, H 1, S 16)
   (Cy 1, H 0, S 1)...(Cy 1, H 0, S 16)(Cy 1, H 1, S 1)...(Cy 1, H 1, S 16)
   ...

   (Cy 76, H 0, S 1)...(Cy 76, H 0, S 16)(Cy 76, H 1, S 1)...(Cy 76, H 1, S 16)
 */

#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include <stdlib.h>
#include"lif_phy.h"
#include "lif_const.h"


void usage(void)
  {
    fprintf (stderr,
    "Usage : lifimage <device> <output file>\n");
    fprintf(stderr, 
    "        lifimage <device> (writes output to standard output)\n");
    exit(1);
  }

int main(int argc, char **argv)
  {
    int input_device;
    int cylinder;
    int head; 
    int sector;
    FILE *output_file;
    unsigned char data[SECTOR_SIZE];

    if((argc != 2) && (argc !=3))
      {
        usage();
      }

    /* open file descriptor for disk drive */
    if((input_device=lif_open_phy_device(argv[1]))==-1)
      {
         fprintf(stderr,"Error opening device %s\n",argv[1]);
         exit(1);
      }

    /* open output file */
    if(argc==3)
      {
        output_file=fopen(argv[2],"w");
       }
    else
      {
        output_file=stdout;
      }

     /* Now start reading the disk */
    for(cylinder=0; cylinder<77; cylinder++)
      {
         lif_seek_phy_device(input_device,cylinder);
         for(head=0; head<2; head++)
           {
             for(sector=1; sector<17; sector++)
               {
                 lif_read_phy_device(input_device,cylinder,head,sector,data);
                 if(fwrite(data,sizeof(char),SECTOR_SIZE,output_file)
                    !=SECTOR_SIZE)
                   {
                     fprintf(stderr,"Error writing output file\n");
                     exit(1);
                   } 
               }
           }
      }
    lif_close_phy_device(input_device);
    if(argc==3)
      {
        fclose(output_file);
      }
    exit(0);
  }
