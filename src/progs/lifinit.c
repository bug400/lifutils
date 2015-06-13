/* lifinit.c -- Initialize a LIF image file */
/* 2014 J. Siebold , and placed under the GPL */

#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include"lif_block.h"
#include"lif_dir_utils.h"
#include "lif_create_entry.h"
#include "lif_const.h"
#ifndef _WIN32
#define O_BINARY 0
#endif


#define DEBUG 0
#define debug_print(fmt, ...) \
   do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)


void usage(void)
  {
    fprintf(stderr,
    "Usage:lifinit lif-image-filename directorysize\n");
    fprintf(stderr,"      lifinit -m Mediumtype lif-image-filename directorysize\n");
    fprintf(stderr,"\n");
    fprintf(stderr, "      -m Medium type (cass | disk | hdrive1) \n");
    exit(1);
  }

int main(int argc, char **argv)
  {
    /* System variables */
    int option; /* Command line option character */
    int physical_flag; /* Option to use a physical device */
    int lif_device; /* Descriptor of input device */
    char *medium= (char *) NULL;
    int dirsize;
    int totalblocks;
    int i;
    
    /* LIF disk values */
    unsigned char sector_data[SECTOR_SIZE]; /* sector */
    unsigned char tmp_data[ENTRY_SIZE]; /* create date and time */

    /* Process command line options */
    optind=1;
    physical_flag=0;

    while ((option=getopt(argc,argv,"pm:x?"))!=-1)
      {
        switch(option)
          {
            case 'm':  medium=optarg;
                       break;
            case 'p' : physical_flag=1;
                       break;
            case '?' : usage();
                       break;
          }
      }

    /* Are the right number of names specified ? */
    if( optind != argc-2  ) 
      {
        /* No, give an error */
        usage();
      }
    /* Is the medium type specified ? */
    if(medium == (char *) NULL)
      {
        usage();
      }
    /* number of directory entries */
    if (sscanf(argv[optind+1],"%d",&dirsize)!= 1) 
      {
        usage();
      }

    /* Open lif device */
#ifdef WIN32
    if((lif_device=lif_open(argv[optind],O_CREAT | O_BINARY | O_TRUNC| O_WRONLY,S_IRUSR | S_IWUSR, physical_flag ))==-1)
#else
    if((lif_device=lif_open(argv[optind],O_CREAT | O_BINARY | O_TRUNC| O_WRONLY,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,physical_flag))==-1)
#endif
      {
        fprintf(stderr,"Error opening %s\n",argv[optind]);
        exit(1);
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
    dirsize= dirsize* ENTRY_SIZE;
    dirsize= dirsize/SECTOR_SIZE + (int) ((dirsize % SECTOR_SIZE) !=0);
    debug_print("Dir size %d\n",dirsize);
    put_lif_int(sector_data+16,4,dirsize);

    /* LIF Version number */
    put_lif_int(sector_data+20,2,1);

    /* Tracks, heads, sectors */
    totalblocks=0;
    if(strcmp(medium,"cass")==0) {
        put_lif_int(sector_data+24,4,2);
        put_lif_int(sector_data+28,4,1);
        put_lif_int(sector_data+32,4,256);
        totalblocks= 512;
    }
    if(strcmp(medium,"disk")==0) {
        put_lif_int(sector_data+24,4,77);
        put_lif_int(sector_data+28,4,2);
        put_lif_int(sector_data+32,4,16);
        totalblocks=2464; /* 16 spare blocks */
    }
    if(strcmp(medium,"hdrive1")==0) {
        put_lif_int(sector_data+24,4,80);
        put_lif_int(sector_data+28,4,2);
        put_lif_int(sector_data+32,4,16);
        totalblocks=2560; 
    }
    if(strcmp(medium,"hdrive2")==0) {
        put_lif_int(sector_data+24,4,125);
        put_lif_int(sector_data+28,4,1);
        put_lif_int(sector_data+32,4,64);
        totalblocks=8000; 
    }
    if(strcmp(medium,"hdrive4")==0) {
        put_lif_int(sector_data+24,4,125);
        put_lif_int(sector_data+28,4,2);
        put_lif_int(sector_data+32,4,64);
        totalblocks=16000; 
    }
    if(strcmp(medium,"hdrive8")==0) {
        put_lif_int(sector_data+24,4,125);
        put_lif_int(sector_data+28,4,4);
        put_lif_int(sector_data+32,4,64);
        totalblocks=32000; 
    }
    if(strcmp(medium,"hdrive16")==0) {
        put_lif_int(sector_data+24,4,125);
        put_lif_int(sector_data+28,4,8);
        put_lif_int(sector_data+32,4,64);
        totalblocks=64000; 
    }
    if(totalblocks==0) usage();

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
    for(i=2;i<dirsize+2;i++)
       lif_write_block(lif_device,i,sector_data);
    /* now write one sector of disk data, all 0xFF */
    lif_write_block(lif_device,dirsize+3,sector_data);
    /* tidy up and quit */
    lif_close(lif_device);
    exit(0);      
  }
