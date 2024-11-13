/* lifrename.c -- rename a file on a LIF disk */
/* 2014 J. Siebold, and placed under the GPL */

#include<stdio.h>
#include<fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "lifutils.h"
#include"lif_block.h"
#include"lif_dir_utils.h"
#include "lif_const.h"

#define DEBUG 0


void lifrename_usage(void)
  {
    fprintf(stderr,"Usage : lifutils lifrename [-l] [-p] LIFIMAGEFILE OLDFILENAME NEWFILENAME\n");
    fprintf(stderr,"        Rename a file in a LIF image file\n");
    fprintf(stderr,"        -l Relax file name checking, allow underscores in new-file names.\n");
    fprintf(stderr,"        -p Rename a file on a floppy disk with a LIF file system.\n");
    fprintf(stderr,"           Note: this option is only supported on LINUX.\n");
    fprintf(stderr,"           Specify the floppy device instead of the lif-image-filename.\n");
    fprintf(stderr,"           See the LIFUTILS tutorial for details.\n");
    fprintf(stderr,"\n");
  }

int lifrename(int argc, char **argv)
  {
    /* System variables */
    int option; /* Command line option character */
    int physical_flag; /* Option to use a physical device */
    int lax; /* Option to relax file name checking */

    int lif_device; /* Descriptor of input device */
    char cmp_name[10]; /* File name to look for */
    char new_name[10]; /* new File name */
    int i;
    
    /* LIF disk values */
    unsigned int dir_start; /* first block of the directory */
    unsigned int dir_length; /* length of directory in blocks */

    /* Directory search values */
    unsigned int dir_end; /* Set at end of directory */
    unsigned int found_file; /* Set when file found */
    unsigned int dir_entry; /* Directory entry within current block */
    unsigned int dir_block; /* Current block offset from start of directory */
    unsigned int abs_entry; /* entry number of file in directory */
    unsigned int file_type; /* file type word */
    unsigned char dir_data[SECTOR_SIZE]; /* Current directory block data */

    /* Process command line options */
    optind=1;
    physical_flag=0;
    lax=0;
    while ((option=getopt(argc,argv,"pl?"))!=-1)
      {
        switch(option)
          {
            case 'p' : physical_flag=1;
                       break;
            case 'l' : lax=1;
                       break;
            case '?' : lifrename_usage();
                       return(RETURN_OK);
          }
      }

    /* Are the right number of names specified ? */
    if( optind != argc-3  )
      {
        /* No, give an error */
        lifrename_usage();
        return(RETURN_ERROR);
      }

    /* Check new file name */
    if(check_filename(argv[optind+2],lax)==0) 
      {
        fprintf(stderr,"Illegal LIF file name\n");
        return(RETURN_ERROR);
      }

    /* Open lif device */
    if((lif_device=lif_open(argv[optind],O_RDWR | O_BINARY,0,physical_flag))==-1)
      {
        fprintf(stderr,"Error opening %s\n",argv[optind]);
        return(RETURN_ERROR);
      }


    /* Now read block 0 to find where the directory is */
    lif_read_block(lif_device,0,dir_data);

    /* Make sure it's a LIF disk */
    if(get_lif_int(dir_data+0,2)!=0x8000)
      {
        fprintf(stderr,"This is not a LIF disk!\n");
        return(RETURN_ERROR);
      }

    /* Find the directory */
    dir_start=get_lif_int(dir_data+8,4);
    dir_length=get_lif_int(dir_data+16,4);

    /* Pad the filename with spaces to enable comparison */
    pad_name(argv[optind+1],cmp_name);

    /* Pad the new filename with spaces */
    pad_name(argv[optind+2],new_name);

    /* Scan the directory to look if new filename already exists */
    dir_end=0;
    found_file=0;
    for(dir_block=0; dir_block<dir_length; dir_block++)
      {
        lif_read_block(lif_device,dir_block+dir_start,dir_data);
        for(dir_entry=0; dir_entry<8; dir_entry++)
          {
            file_type=get_lif_int(dir_data+(dir_entry<<5)+10,2);
            if(file_type==0) { continue; } /* Skip deleted files */ 
            if(file_type==0xFFFF)
              {
                /* End of directory */
                dir_end=1;
                break;
              }
            if(compare_names((char *)dir_data+(dir_entry<<5),new_name))
              {
                /* Found the file */
                found_file=1;
                break;
              }
          }
        if(dir_end || found_file) { break; }; /* Quit at end or if file found */
      }

    if(found_file)
      {
        /* Give file already exists error */
        fprintf(stderr,"File %s already exists\n",argv[optind+2]);
        return(RETURN_ERROR);
      }

    /* Change file name */

    /* Scan the directory */
    dir_end=0;
    found_file=0;
    abs_entry=-1;
    for(dir_block=0; dir_block<dir_length; dir_block++)
      {
        lif_read_block(lif_device,dir_block+dir_start,dir_data);
        for(dir_entry=0; dir_entry<8; dir_entry++)
          {
            file_type=get_lif_int(dir_data+(dir_entry<<5)+10,2);
            abs_entry++;
            if(file_type==0) { continue; } /* Skip deleted files */ 
            if(file_type==0xFFFF)
              {
                /* End of directory */
                dir_end=1;
                break;
              }
            if(compare_names((char *)dir_data+(dir_entry<<5),cmp_name))
              {
                /* Found the file */
                found_file=1;
                break;
              }
          }
        if(dir_end || found_file) { break; }; /* Quit at end or if file found */
      }

    if(!found_file)
      {
        /* Give file not found error */
        fprintf(stderr,"File %s not found\n",argv[optind+1]);
        return(RETURN_ERROR);
      }

    /* Change file name */
    for(i=0;i< NAME_LEN; i++) 
       dir_data[(dir_entry <<5)+i]= new_name[i];

    /* Actually delete the directory entry */
   lif_write_dir_entry(lif_device,dir_start,abs_entry, dir_data+(dir_entry<<5));

    /* tidy up and quit */
    lif_close(lif_device);
    return(RETURN_OK);
  }
