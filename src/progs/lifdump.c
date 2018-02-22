/* lifdump.c -- dump a image file back to a physical LIF disk */
/* 2000 A. R. Duell, and placed under the GPL */

/* See lifimage.c for a description of the image file, which is 
   basically just the data blocks in the 'obvious' order */

#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include <stdlib.h>
#include"lif_phy.h"
#include "lif_const.h"


void usage(void)
  {
    fprintf(stderr,
    "Usage : lifdump <input file> <device>\n");
    fprintf(stderr,
    "        lifdump <device> (input comes from standard input)\n");
    exit(1);
  }

int main(int argc, char **argv)
  {
    int output_device;
    int cylinder, head, sector;
    FILE *input_file;
    unsigned char data[SECTOR_SIZE];

    if((argc!=2) && (argc!=3))
      {
        usage();
      }

    /* open descriptor for output disk drive */
    if((output_device=lif_open_phy_device(argv[argc-1]))==-1)
      {
        fprintf(stderr,"Error opening device %s\n",argv[argc-1]);
        exit(1);
      }

    /* open input file */
    if(argc==3)
      {
        input_file=fopen(argv[1],"r");
      }
    else
      {
        input_file=stdin;
      }

    /* Now start writing to the disk */
    for(cylinder=0; cylinder<77; cylinder++)
      {
        lif_seek_phy_device(output_device,cylinder);
        for(head=0; head<2; head++)
          {
            for(sector=1; sector<17; sector++)
              {
                if(fread(data,sizeof(char),SECTOR_SIZE,input_file)
                   !=SECTOR_SIZE)
                  {
                    fprintf(stderr,"Error reading input file\n");
                    exit(1);
                  }
                lif_write_phy_device(output_device,cylinder,head,sector,data);
              }
          }
      }
    lif_close_phy_device(output_device);
    if(argc==3)
      {
        fclose(input_file);
      }
    exit(0);
  }
