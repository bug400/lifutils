/* lif_img.c -- read/write a block (specified by logical block number) to/from
                  a lif disk */ 
/*  2000,2015 A. R. Duell, J. Siebold and placed under the GPL */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "lif_const.h"

#define DEBUG 0
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

/* open lif image file */
int lif_open_img_file(char *filename, int flags, int mode)
  {
      int iret;

      iret=open(filename,flags,mode);
      /* the error handling is done by the caller, so output system error message only */
      if (iret== -1) 
        {
          fprintf(stderr,"%s\n",strerror(errno));
        }
      return(iret);
  }
/* close lif image file */
void lif_close_img_file(int descriptor)
  {
      close(descriptor);
  }

/* truncate a lif image file */
void lif_truncate_img_file(int lif_file)
  {
      if(ftruncate(lif_file,0))
       {
          fprintf(stderr,"Error truncating file (%s)\n",strerror(errno));
          exit(1);
       }
  }

/* Read one block from an lif image file */
void lif_read_img_block(int input_file, int block, unsigned char *data)
  {
    off_t seek_ret;
    ssize_t read_ret;

    /* Go to the right block in the file */
    seek_ret=lseek(input_file,(off_t) (SECTOR_SIZE*block),SEEK_SET);
    if (seek_ret == (off_t) -1) 
      {
        fprintf(stderr,"Error seeking to block %d. (%s)\n",block,strerror(errno));
        exit(1);
      }
    /* Read it */
    debug_print("read block %d\n",block); 
    read_ret=read(input_file,data,(size_t) SECTOR_SIZE);
    if (read_ret== (ssize_t) -1) 
      {
        fprintf(stderr,"Error reading block %d from file. (%s)\n",block,strerror(errno));
        exit(1);
      }
    if (read_ret != SECTOR_SIZE) 
      {
        fprintf(stderr,"Premature end of sector %d. %ld bytes read.\n", block, read_ret);
        exit(1);
      }
  }

/* Write one block to an lif image file */
void lif_write_img_block(int output_file, int block, unsigned char *data)
  {
    off_t seek_ret;
    ssize_t write_ret;

    /* Go to the right block in the file */
    seek_ret=lseek(output_file,(off_t) (SECTOR_SIZE*block),SEEK_SET);
    if (seek_ret == (off_t) -1) 
      {
        fprintf(stderr,"Error seeking to block %d. (%s)\n",block,strerror(errno));
        exit(1);
      }

    /* Write it */
    debug_print("write to block %d\n",block); 
    write_ret=write(output_file,data,SECTOR_SIZE);
    if(write_ret == (ssize_t) -1)
      {
        fprintf(stderr,"Error writing block %d from file (%s)\n",block,strerror(errno));
        exit(1);
      }
    if (write_ret != SECTOR_SIZE) 
      {
        fprintf(stderr,"Premature end of sector %d. %ld bytes written.\n", block, write_ret);
        exit(1);
      }
  }
