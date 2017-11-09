/* lif_block.c -- generic i/o layer for lif disk or image file */
/*  2000, 2015 A. R. Duell, J. Siebold and placed under the GPL */

#include<stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "lif_img.h"
#include "lif_phy.h"
#include "lif_const.h"

#define DEBUG 0
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

static int p_flag;

int lif_open(char * filename,int flags,int mode, int physical_flag)
  {
   int fileno;

   /* open file or device */
   p_flag= physical_flag;
   if (p_flag)
      {
        fileno=lif_open_phy_device(filename, flags);
      }
    else
      {
        fileno=lif_open_img_file(filename,flags, mode);
      }
    return(fileno);
  }

void lif_close(int fileno)
  {
   /* close file or device */
   if (p_flag)
      {
        lif_close_phy_device(fileno);
      }
    else
      {
        lif_close_img_file(fileno);
      }
  }

void lif_read_block(int input_file, int block, unsigned char *data)
  {
    /* Read one block */
   if (p_flag)
      {
        lif_read_phy_block(input_file,block,data);
      }
    else
      {
        lif_read_img_block(input_file,block,data);
      }

  }

void lif_truncate(int fileno)
  {
   if (!p_flag) lif_truncate_img_file(fileno);
  }

void lif_write_block(int output_file, int block, unsigned char *data)
  {
    /* Write one block */
   if (p_flag)
      {
        lif_write_phy_block(output_file,block,data);
      }
    else
      {
        lif_write_img_block(output_file,block,data);
      }

  }


void lif_write_dir_entry(int output_file, int dir_start, int entry, unsigned char * dir_entry)

   {
     int blocknum, n, offset,i;
     unsigned char block[SECTOR_SIZE];

     debug_print("directory entry %d\n", entry);
     n= (SECTOR_SIZE/ ENTRY_SIZE);
     blocknum= dir_start + ( entry / n);
     debug_print("block %d\n",blocknum); 

     /* read sector of the entry */
     lif_read_block(output_file,blocknum, block);

     /* poke entry into block */
     offset= entry - ((int) (entry /n)) * n;
     offset= offset* ENTRY_SIZE;
     debug_print("directory offset %d\n",offset);
     for (i=0; i< ENTRY_SIZE; i++)
        block[i+offset]= dir_entry[i];
     
     /* write block */
     lif_write_block(output_file,blocknum, block);
   }

