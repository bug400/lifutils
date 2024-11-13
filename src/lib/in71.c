/* in71.c -- read a file in from an HP71 (e.g. over an RS232 interface) */
/* 2000 A. R. Duell, and placed under the GPL */

/* The HP71B can copy files not only to mass storage devices but also to 
   any other HPIL device, for example an interface to another computer.
   The data sent consists of the 32 byte directory entry for the file 
   followed by sufficient 256 byte blocks to contain the file data (the
   end of the last block is filled with garbage). By stripping off the 
   directory entry and truncating the last block appropriately, the
   original file can be recovered. */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"
#include "lifutils.h"

#include"lif_dir_utils.h"
#include "lif_const.h"

int in71_get_length(unsigned char block_flag)
/* Get the directory entry from standard input, display the approrpiate
   parts to standard error, and return the number of bytes to copy */
  {
    unsigned char dir_entry[ENTRY_SIZE]; /* directory entry */
    char file_type[10]; /* File type string */
    int data_length,block_length; /* File lengths */
    int i;

    /* Read in the directory entry */
    if(fread(dir_entry,sizeof(unsigned char),ENTRY_SIZE,stdin)!=ENTRY_SIZE)
      {
        fprintf(stderr,"Error reading directory entry\n");
        return(RETURN_ERROR);
      }

    /* Decode it */
    data_length=file_length(dir_entry,file_type);
    block_length=get_lif_int(dir_entry+16,4)*BLOCK_SIZE;

    /* Display file information on stnadard error */
    fprintf(stderr,"Reading : ");
    i=0;
    /* display filename */
    while((i<10) && dir_entry[i]!=' ')
      {
        fputc(dir_entry[i++],stderr);
      }
    fprintf(stderr," %s %d/%d bytes\n",file_type,data_length,block_length);

    /* return appropriate length */
    return(block_flag?block_length:data_length);
  }

int in71_copy_data(int length)
/* Copy data from standard input to standard output, always reading 256 byte
   blocks from standard input */
  {
    int full_blocks,leftover_bytes;
    int i; /* loop counter */
    unsigned char data_block[BLOCK_SIZE]; /* One block of the file */

    if((full_blocks=length/BLOCK_SIZE)) /* Yes, this is an assignment */
      {
        for(i=0; i<full_blocks; i++)
          {
            /* Copy the complete data blocks */
            if(fread(data_block,sizeof(unsigned char),BLOCK_SIZE,stdin)
              !=BLOCK_SIZE)
              {
                fprintf(stderr,"Error reading data block\n");
                return(RETURN_ERROR);
              }
            fwrite(data_block,sizeof(unsigned char),BLOCK_SIZE,stdout);
          }
      }
    if((leftover_bytes=length%BLOCK_SIZE)) /* And so is this */
      {
        /* Copy the last partial block */
        if(fread(data_block,sizeof(unsigned char),BLOCK_SIZE,stdin)!=BLOCK_SIZE)
          {
            fprintf(stderr,"Error reading last data block\n");
            return(RETURN_ERROR);
          }
        fwrite(data_block,sizeof(unsigned char),leftover_bytes,stdout);
      }
    return(RETURN_OK);
  }

void in71_usage(void)
  {
    fprintf(stderr,"Usage : lifutils in71 [-b] < input device > output file\n");
    fprintf(stderr,"        Read a program from a HP-71B\n");
    fprintf(stderr,"        -b copies entire last block\n");
    fprintf(stderr,"\n");
  }

int in71(int argc, char **argv)
  {
    int option; /* command line option character */
    unsigned char block_flag=0; /* Copy entire last block? */
    int length; /* Number of bytes to copy */


    optind=1;
    while((option=getopt(argc,argv,"b?"))!=-1)
      {
        switch(option)
          {
            case 'b' : block_flag=1;
                       break;
            case '?' : in71_usage();
                       return(RETURN_OK);
          }
      }
   if(optind!=argc) {
       in71_usage();
       return(RETURN_ERROR);
    }


    SETMODE_STDIN_BINARY;
    SETMODE_STDOUT_BINARY;
    /* Get the directory entry and copy the file */
    length=in71_get_length(block_flag);
    if(length<0) return(RETURN_ERROR);
    if(in71_copy_data(length)<0) return(RETURN_ERROR);
    else return(RETURN_OK);
  }

