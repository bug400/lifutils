/* lifpurge.c -- Get a file from a LIF disk */
/* 2014 J. Siebold, and placed under the GPL */

#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include"lif_block.h"
#include"lif_dir_utils.h"
#include "lif_const.h"
#include "lif_create_entry.h"
#ifndef _WIN32
#define O_BINARY 0
#endif


#define DEBUG 1
#define debug_print(fmt, ...) \
   do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)


void usage(void)
  {
    fprintf(stderr,
    "Usage:liffix lif-image-filename \n");
    fprintf(stderr,"      liffix -m Mediumtype lif-image-filename\n");
    fprintf(stderr,"\n");
    fprintf(stderr, "      -m Medium type (cass | disk | hdrive1) \n");
    exit(1);
  }

int main(int argc, char **argv)
  {
    /* System variables */
    int option; /* Command line option character */
    int lif_device; /* Descriptor of input device */
    int physical_flag; /* Option to use a physical device */
    char *medium= (char *) NULL;
    int totalblocks;
    int dir_length;
    int dir_block;
    int start_block;
    int num_blocks;
    int end_block;
    int maxblock;
    int dir_entry;
    int dir_end;
    int dir_start;
    int file_type;
    
    /* LIF disk values */
    unsigned char sector_data[SECTOR_SIZE]; /*  start sector */
    unsigned char dir_data[SECTOR_SIZE]; /* dir sector */

    /* Process command line options */
    optind=1;
    physical_flag=0;
    while ((option=getopt(argc,argv,"pm:x?"))!=-1)
      {
        switch(option)
          {
            case 'p' : physical_flag=1;
                        break;
            case 'm':  medium=optarg;
                       break;
            case '?' : usage();
                       break;
          }
      }

    /* Are the right number of names specified ? */
    if( optind != argc-1  ) 
      {
        /* No, give an error */
        usage();
      }
    /* Is the medium type specified ? */
    if(medium == (char *) NULL)
      {
        usage();
      }
    /* Open lif device */
    if((lif_device=lif_open(argv[optind],O_RDWR | O_BINARY,0,physical_flag))==-1)
      {
        fprintf(stderr,"Error opening %s\n",argv[optind]);
        exit(1);
      }
    /* Now read block 0 to find where the directory is */
    lif_read_block(lif_device,0,sector_data);

    /* Make sure it's a LIF disk */
    if(get_lif_int(sector_data+0,2)!=0x8000)
      {
        fprintf(stderr,"This is not a LIF disk!\n");
        exit(1);
      }
    /* Find where the directory is */
    dir_start=get_lif_int((sector_data+8),4);
    dir_length=get_lif_int((sector_data+16),4);

    /* Now scan the directory to find the last block used by the filesystem */
    maxblock=0;
    for(dir_block=0; dir_block<dir_length; dir_block++)
      {
         dir_end=0;
         lif_read_block(lif_device,dir_block+dir_start,dir_data);
         for(dir_entry=0; dir_entry<8; dir_entry++)
           {
             file_type=get_lif_int((dir_data+(dir_entry<<5)+10),2);
             if(file_type==0) { continue; } /* Skip over deleted files */
             if(file_type==0xffff)
               {
                 /* This is the end of the directory */
                 dir_end=1;
                 break;
               }
             start_block=get_lif_int(dir_data+(dir_entry<<5)+12,4);
             num_blocks=get_lif_int(dir_data+(dir_entry<<5)+16,4);
             end_block= start_block+num_blocks;
             if(end_block > maxblock) maxblock= end_block;
           }
         if(dir_end) { break; } /* Quit at end of directory */
      }

    /* Tracks, heads, sectors */
    totalblocks=0;
    if(strcmp(medium,"cass")==0) {
        put_lif_int(sector_data+24,4,2);
        put_lif_int(sector_data+28,4,1);
        put_lif_int(sector_data+32,4,256);
        if ((sector_data[18] & 0x40) != 0) sector_data[18] &= ~ 0x40;
        *(sector_data+18)=0;
        totalblocks= 512;
    }
    if(strcmp(medium,"disk")==0) {
        put_lif_int(sector_data+24,4,77);
        put_lif_int(sector_data+28,4,2);
        put_lif_int(sector_data+32,4,16);
        if ((sector_data[18] & 0x40) != 0) sector_data[18] &= ~ 0x40;
        totalblocks=2464; /* 16 spare blocks */
    }
    if(strcmp(medium,"hdrive1")==0) {
        put_lif_int(sector_data+24,4,80);
        put_lif_int(sector_data+28,4,2);
        put_lif_int(sector_data+32,4,16);
        *(sector_data+18)=0;
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
    debug_print("Last block used %d\n",maxblock);
    debug_print("Total blocks of medium %d\n",totalblocks);
    if(totalblocks < maxblock) 
     {
        fprintf(stderr,"Medium too small\n");
        lif_close(lif_device);
        exit(1);
      }
    /* set LIF Version number */
    put_lif_int(sector_data+20,2,1);

    /* Now write block 0  */
    lif_write_block(lif_device,0,sector_data);
    /* tidy up and quit */
    lif_close(lif_device);
    exit(0);      
  }
