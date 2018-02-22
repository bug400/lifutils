/* lif_phy.c -- interface to physical LIF disks (LINUX) */
/* 2000, 2015 A. R. Duell, J. Siebold and placed under the GPL */

/* This file contains machine-dependant functions to access sectors on
   a real LIF disk inserted in a PC floppy drive. This versions is for
   Linux */

/* It appears that an HP LIF disk (as used on the HP71/9114, etc) consists
   of 77 cylinders (numbered 0-76), 2 sides (0 and 1) and 16 sectors/track
   (1-16), 256 bytes/sector. Double density at the standard (256 kbps) data
   rate */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/fd.h>
#include <linux/fdreg.h>
#include "lif_phy.h"
#include "lif_const.h"

/* Data rate selection code for 250kbps */
#define RATE250 2

/* Read/write a single double density sector functions for the 8272 
   disk controller. These are not in fdreg.h */
#define FD_DD_WRITE 0x45
#define FD_DD_READ 0x46


static int current_cylinder;


int lif_open_phy_device(char * devicename)
  {
    int device;

    device=open(devicename,3,0);
    if(device != -1) 
    {
       /* Move the drive head to cylinder 0 and set current_cylinder */
       lif_recalibrate_phy_device(device);
       current_cylinder=0;
    }
    return(device);
  }

void lif_close_phy_device(descriptor)
  {
     close(descriptor);
  }

void lif_read_phy_block(int input_device, int block, unsigned char *data)
  {
    /* Read one block from a physical LIF disk */
    int cylinder, head, sector;

    /* Calculate where this block is on the disk */
    cylinder=block/32;
    head=(block/16)%2;
    sector=(block%16)+1;

    /* If we're not on the right cylinder, go there */
    if(cylinder!=current_cylinder)
      {
        lif_seek_phy_device(input_device,cylinder);
        current_cylinder=cylinder;
      }

    /* Now read the block */
    lif_read_phy_device(input_device,cylinder,head,sector,data);
  }

void lif_write_phy_block(int output_device, int block, unsigned char *data)
  {
    /* Read one block from a physical LIF disk */
    int cylinder, head, sector;
  
    /* Calculate where this block is on the disk */
    cylinder=block/32;
    head=(block/16)%2;
    sector=(block%16)+1;
    
    /* If we're not on the right cylinder, go there */
    if(cylinder!=current_cylinder)
      {
        lif_seek_phy_device(output_device,cylinder);
        current_cylinder=cylinder;
      }

    /* Now write the block */
    lif_write_phy_device(output_device,cylinder,head,sector,data);
  }


void lif_recalibrate_phy_device(int device)
  {
    struct floppy_raw_cmd cmd;
    struct floppy_struct  floppy;
    /* clear floppy config */
    
    if(ioctl(device,FDCLRPRM,NULL)<0)
      {
         fprintf(stderr,"Error on resetting floppy parameters\n");
         fprintf(stderr,"error= %s (%d)\n",strerror(errno),errno);
         exit(1);
       }
    /* configure floppy */
    floppy.size= 2464; /* total number of sectors */
    floppy.sect= 16;   /* sectors per track */
    floppy.head=2;     /* nr of heads */
    floppy.track=77;   /* nr of tracks */
    floppy.stretch=0;  /* no double track steps, no swap sides, first sect=0 */
    if(ioctl(device,FDSETPRM,&floppy)<0)
      {
         fprintf(stderr,"Error on configuring floppy parameters\n");
         fprintf(stderr,"error= %s (%d)\n",strerror(errno),errno);
         exit(1);
       }
    cmd.data=NULL;  /* pointer to data buffer */
    cmd.length=0;   /* length of DMA transfer */
    cmd.rate=RATE250; /* data rate = 250kbps */
    cmd.flags=FD_RAW_INTR;
    cmd.cmd[0]=FD_RECALIBRATE; /* position head to track 0 */
    cmd.cmd[1]=0;
    cmd.cmd_count=2;  /* two bytes in the command */
    if(ioctl(device,FDRAWCMD,&cmd)<0)
      {
         fprintf(stderr,"Error on recalibrate\n");
         fprintf(stderr,"error= %s (%d)\n",strerror(errno),errno);
         exit(1);
       }
  }

void lif_seek_phy_device(int device, int cylinder)
  {
    struct floppy_raw_cmd cmd;

    cmd.data=NULL;   /* No data to transfer */
    cmd.length=0;
    cmd.rate=RATE250; /* 250kbps data rate */
    cmd.flags=FD_RAW_INTR;
    cmd.cmd[0]=FD_SEEK;  /* Seek to a cylinder */
    cmd.cmd[1]=0;
    cmd.cmd[2]=cylinder;  /* Cylinder to go to */
    cmd.cmd_count=3;
    if (ioctl(device,FDRAWCMD,&cmd)<0)
      {
         fprintf(stderr,"Error on seek to cylinder %d\n",cylinder);
         fprintf(stderr,"error= %s (%d)\n",strerror(errno),errno);
         exit(1);
      }
  }

void lif_read_phy_device(int device, int cylinder, int head, int sector, 
              unsigned char *data)
  {
    struct floppy_raw_cmd cmd;

    cmd.data=data;  /* Data buffer to transfer */
    cmd.length=SECTOR_SIZE; /* length of data */
    cmd.rate=RATE250; /* 250 kbps */
    cmd.flags=FD_RAW_INTR | FD_RAW_READ; /* Set up DMA, etc */
    cmd.cmd[0]=FD_DD_READ; /* read command */
    cmd.cmd[1]=head?4:0; /* Head select */
    cmd.cmd[2]=cylinder; /* Cylinder value (to check with header) */
    cmd.cmd[3]=head?1:0; /* Head value (to check with header) */
    cmd.cmd[4]=sector; /* Sector to search for */
    cmd.cmd[5]=1; /* 256 byte MFM sectors */
    cmd.cmd[6]=16; /* last sector number on a track */
    cmd.cmd[7]=32; /* gap length */
    cmd.cmd[8]=0xFF; /* 256 bytes, but shouldn't be needed */
    cmd.cmd_count=9;
    if ((ioctl(device,FDRAWCMD,&cmd)<0) || (cmd.reply[0] & 0xC0))
      {
        fprintf(stderr,"Error reading cylinder %d, head %d, sector %d\n",
                cylinder,head,sector);
         fprintf(stderr,"error= %s (%d)\n",strerror(errno),errno);
        exit(1);
      }
  }

void lif_write_phy_device(int device, int cylinder, int head, int sector, 
               unsigned char *data)
  {
    struct floppy_raw_cmd cmd;

    cmd.data=data;  /* Data buffer to transfer */
    cmd.length=SECTOR_SIZE; /* length of data */
    cmd.rate=RATE250; /* 250 kbps */
    cmd.flags=FD_RAW_INTR | FD_RAW_WRITE; /* Set up DMA, etc */
    cmd.cmd[0]=FD_DD_WRITE; /* write command */
    cmd.cmd[1]=head?4:0; /* Head select */
    cmd.cmd[2]=cylinder; /* Cylinder value (to check with header) */
    cmd.cmd[3]=head?1:0; /* Head value (to check with header) */
    cmd.cmd[4]=sector; /* Sector to search for */
    cmd.cmd[5]=1; /* 256 byte MFM sectors */
    cmd.cmd[6]=16; /* last sector number on a track */
    cmd.cmd[7]=32; /* gap length */
    cmd.cmd[8]=0xFF; /* 256 bytes, but shouldn't be needed */
    cmd.cmd_count=9;
    if ((ioctl(device,FDRAWCMD,&cmd)<0) || (cmd.reply[0] & 0xC0))
      {
        fprintf(stderr,"Error writing cylinder %d, head %d, sector %d\n",
                cylinder,head,sector);
        fprintf(stderr,"error= %s (%d)\n",strerror(errno),errno);
        exit(1);
      }
  }

