/* lif_phy.c -- interface to physical LIF disks (dummy) */
/* J. Siebold and placed under the GPL */

/* This file contains machine-dependant functions to access sectors on
   a real LIF disk inserted in a PC floppy drive. This versions is for
   operating systems that do not support physical floppy access and
   contains dummy routines */


#include<stdio.h>

int lif_open_phy_device(devicename)
  {
    int device= -1;
    fprintf(stderr,"Low level floppy disc access is not supported on this platform\n");

    return(device);
  }

void lif_close_phy_device(descriptor)
  {
  }

void lif_read_phy_block(int input_device, int block, unsigned char *data)
  {
  }

void lif_write_phy_block(int output_device, int block, unsigned char *data)
  {
  }

void lif_seek_phy_device(int device, int cylinder)
  {
  }

void lif_read_phy_device(int device, int cylinder, int head, int sector,
              unsigned char *data)
  {
  }

void lif_write_phy_device(int device, int cylinder, int head, int sector,
               unsigned char *data)
  {
  }
