/* lifinit.c -- Initialize a LIF image file */
/* 2014 J. Siebold , and placed under the GPL */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include "config.h"
#include "lifutils.h"
#include"lif_block.h"
#include"lif_dir_utils.h"
#include "lif_create_entry.h"
#include "lif_const.h"


#define DEBUG 0


void lifinit_usage(void)
  {
    fprintf(stderr,"Usage : lifutils lifinit [-z] [-p] -m MEDIUMTYPE LIFIMAGEFILE DIRECTORYSIZE\n");
    fprintf(stderr,"        Initialize a LIF image file\n");
    fprintf(stderr,"        -m MEDIUMTYPE (cass | disk | hdrive1/2/4/8/16) \n");
    fprintf(stderr,"        -z Initialize the data are with zeros) \n");
    fprintf(stderr,"        -p Initialize a LIF file system on a floppy disk.\n");
    fprintf(stderr,"           Note: this option is only supported on LINUX.\n");
    fprintf(stderr,"           Specify the floppy device instead of the lif-image-filename.\n");
    fprintf(stderr,"           See the LIFUTILS tutorial for details.\n");
    fprintf(stderr,"\n");
  }

int lifinit(int argc, char **argv)
  {
    /* System variables */
    int option; /* Command line option character */
    int physical_flag; /* Option to use a physical device */
    int zero_data; /* Option to fill the data area with zeros */
    int lif_device; /* Descriptor of input device */
    char *medium= (char *) NULL;
    int dirsize;    /* number of directory entries */
    int dirsize_blocks; /* number of blocks for directory */
    int first_data_block; /* first block of data area */
    int totalblocks; /* total number of disk blocks */
    int tracks, heads, sectors; /* medium geometry */
    int i, temp;
    
    /* LIF disk values */
    unsigned char sector_data[SECTOR_SIZE]; /* sector */
    unsigned char tmp_data[ENTRY_SIZE]; /* create date and time */

    /* Process command line options */
    optind=1;
    physical_flag=0;
    zero_data=0;

    while ((option=getopt(argc,argv,"pm:xz?"))!=-1)
      {
        switch(option)
          {
            case 'm':  medium=optarg;
                       break;
            case 'p' : physical_flag=1;
                       break;
            case 'z' : zero_data=1;
                       break;
            case '?' : lifinit_usage();
                       return(RETURN_OK);
          }
      }

    /* Are the right number of names specified ? */
    if( optind != argc-2  ) 
      {
        /* No, give an error */
        lifinit_usage();
        return(RETURN_ERROR);
      }
    /* Is the medium type specified ? */
    if(medium == (char *) NULL)
      {
        lifinit_usage();
        return(RETURN_ERROR);
      }
    /* number of directory entries */
    if (sscanf(argv[optind+1],"%d",&dirsize)!= 1) 
      {
        lifinit_usage();
        return(RETURN_ERROR);
      }
    /* size of directory in blocks */
    temp= dirsize* ENTRY_SIZE;
    dirsize_blocks= temp/SECTOR_SIZE + (int) ((temp % SECTOR_SIZE) !=0);

    /* Tracks, heads, sectors */
    totalblocks=0;
    if(strcmp(medium,"cass")==0) {
        tracks=2;
        heads=1;
        sectors=256;
        totalblocks= 512;
    }
    if(strcmp(medium,"disk")==0) {
        tracks=77;
        heads=2;
        sectors=16;
        totalblocks=2464; 
    }
    if(strcmp(medium,"hdrive1")==0) {
        tracks=80;
        heads=2;
        sectors=16;
        totalblocks=2560; 
    }
    if(strcmp(medium,"hdrive2")==0) {
        tracks=125;
        heads=1;
        sectors=64;
        totalblocks=8000; 
    }
    if(strcmp(medium,"hdrive4")==0) {
        tracks=125;
        heads=2;
        sectors=64;
        totalblocks=16000; 
    }
    if(strcmp(medium,"hdrive8")==0) {
        tracks=125;
        heads=4;
        sectors=64;
        totalblocks=32000; 
    }
    if(strcmp(medium,"hdrive16")==0) {
        tracks=125;
        heads=8;
        sectors=64;
        totalblocks=64000; 
    }
    if(totalblocks==0) {
       lifinit_usage();
       return(RETURN_ERROR);
    }

    /* directory size too large? */
    if (dirsize_blocks >  totalblocks / 3) {
       fprintf(stderr,"directory size too large\n");
       return(RETURN_ERROR);
    }
    first_data_block= dirsize_blocks+2;
    debug_print("Dir size %d\n",dirsize);
    debug_print("Dir size (blocks) %d\n",dirsize_blocks);
    debug_print("Tracks %d\n",tracks);
    debug_print("Heads %d\n",heads);
    debug_print("Sectors %d\n",sectors);
    debug_print("First data block %d\n",first_data_block);
    debug_print("Total blocks %d\n",totalblocks);

    /* Open lif device */
    if((lif_device=lif_open(argv[optind],O_CREAT | O_BINARY | O_TRUNC| O_WRONLY,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,physical_flag))==-1)
      {
        fprintf(stderr,"Error opening %s\n",argv[optind]);
        return(RETURN_ERROR);
      }
 
    /* now fill first block */
    for(i=0;i<SECTOR_SIZE;i++) sector_data[i]= 0x0;

    /* LIF magic */
    put_lif_int(sector_data+0,2,0x8000);

    /* blank label */
    for(i=2;i<8;i++) sector_data[i]=' ';

    /* directory start block */
    put_lif_int(sector_data+8,4,2);

    /* length of directory in blocks */
    put_lif_int(sector_data+16,4,dirsize_blocks);

    /* LIF Version number */
    put_lif_int(sector_data+20,2,1);

    /* write tracks, heads, sectors */
    put_lif_int(sector_data+24,4,tracks);
    put_lif_int(sector_data+28,4,heads);
    put_lif_int(sector_data+32,4,sectors);

    /* date and time are createt in an directory entry structure */ 
    put_time(tmp_data);
    for(i=36;i<40;i++) sector_data[i]= tmp_data[i-16];

    /* Now write block 0  */
    lif_write_block(lif_device,0,sector_data);

    /* Now write block 1 , all zeros */
    for(i=0;i<SECTOR_SIZE;i++) sector_data[i]=0x0;
    lif_write_block(lif_device,1,sector_data);

    /* Now write empty directory, all 0xFF */
    for(i=0;i<SECTOR_SIZE;i++) sector_data[i]=0xFF;
    for(i=2;i<first_data_block;i++) {
       debug_print("init directory area block %d\n",i);
       lif_write_block(lif_device,i,sector_data);
    }

    /* now write one sector of disk data, all 0xFF */
    debug_print("init one disk block %d\n",first_data_block);
    lif_write_block(lif_device,first_data_block,sector_data);

    /* zero data area if requested */
    if (zero_data)
       {
       for(i=0;i<SECTOR_SIZE;i++) sector_data[i]=0x0;
       i=first_data_block+1; /* first data after already initialized block */
       while(i< totalblocks)
          {
          debug_print("zero disk data block %d\n",i);
          lif_write_block(lif_device,i,sector_data);
          i+=1;
          }
       }
    /* tidy up and quit */
    lif_close(lif_device);
    return(RETURN_OK);
  }
