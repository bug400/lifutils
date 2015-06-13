/* lif_img.c -- read/write a block (specified by logical block number) to/from
                  a lif disk */ 
/*  2000,2015 A. R. Duell, J. Siebold and placed under the GPL */

#include<stdio.h>
#include<unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include<fcntl.h>
#include "lif_const.h"

#define DEBUG 0
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

int lif_open_img_file(char *filename, int flags, int mode)
  {
      return(open(filename,flags,mode));
  }

void lif_close_img_file(int descriptor)
  {
      close(descriptor);
  }

/* truncate a file */
void lif_truncate_img_file(int lif_file)
  {
      if(ftruncate(lif_file,0))
       {
          fprintf(stderr,"Error truncating file (%s)\n",strerror(errno));
          exit(1);
       }
  }

void lif_read_img_block(int input_file, int block, unsigned char *data)
  {
    /* Read one block from an image file */

    /* Go to the right block in the file */
    lseek(input_file,(off_t) (SECTOR_SIZE*block),SEEK_SET);
    /* Read it */
    if(read(input_file,data,(size_t) SECTOR_SIZE)!=SECTOR_SIZE)
      {
        fprintf(stderr,"Error reading block %d from file. (%s)\n",block,strerror(errno));
        exit(1);
      }
  }


void lif_write_img_block(int output_file, int block, unsigned char *data)
  {
    /* Write one block to an image file */

    /* Go to the right block in the file */
    lseek(output_file,(off_t)(SECTOR_SIZE*block),SEEK_SET);
    /* Write it */
    debug_print("write to block %d\n",block); 
    if(write(output_file,data,SECTOR_SIZE)!= SECTOR_SIZE)
      {
        fprintf(stderr,"Error writing block %d from file (%s)\n",block,strerror(errno));
        exit(1);
      }
  }
