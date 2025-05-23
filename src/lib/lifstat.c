/* lifstat.c -- Display status information of a LIF disk */
/* 2001 A. R. Duell, and placed under the GPL */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"
#include "lifutils.h"
#include "lif_block.h"
#include "lif_dir_utils.h"
#include "lif_const.h"

/* parameters of the LIF disk */
#define SPT 16
#define HEADS 2
#define CYLINDERS 77
#define TOTAL_BLOCKS 2464


void lifstat_print_block_no(unsigned int block_no)
/* Print the block number, and it's address (cylinder/head/sector) to 
   standard output */
  {
    printf("%d (%d/%d/%d)",
            block_no,
            (int) block_no/(HEADS*SPT),
            (int) (block_no/SPT)%HEADS,
            (block_no%SPT)+1);
  }

int lifstat_lif_status(int input_file, int block_flag, unsigned 
                int block_no)
/* Print various status items for the given LIF disk (or image) to standard
   output. If block_flag is false, then a sumary of data for the entire disk
   is produced. If block_flag is true, then the name of the file 
   containing block_no is output */
  {
    unsigned int dir_start; /* first directory block */
    unsigned int dir_length; /* size of directory */
    unsigned int dir_end; /* Set at end of directory search */
    unsigned int found; /* Set if given block found */
    unsigned int dir_entry; /* directory entry in current block */
    unsigned int dir_block; /* current directory block (offset from dir_start) */
    unsigned int file_type; /* current file type */
    unsigned int file_start; /* start block of current file */
    unsigned int file_length; /* length of current file */
    unsigned int last_block; /* last used block on the disk */
    unsigned int surfaces;  /* no of disk surfaces in lif header */
    unsigned int tracks; /* no of tracks in lif header */
    unsigned int blocks; /* no of blocks in lif header */
    unsigned int totalsize; /* size of medium in blocks */

    int i; /* loop index */
    char c; /* filename character */
    unsigned char data[SECTOR_SIZE]; /* disk block */

    last_block=0;

    /* Handle the fixed cases */
    if((block_flag)&&(block_no==0))
      {
        lifstat_print_block_no(block_no);
        printf(" : Volume Label\n");
        return(RETURN_OK);
      }
    if((block_flag)&&(block_no==1))
      {
        lifstat_print_block_no(block_no);
        printf(" : System 3000 block\n");
        return(RETURN_OK);
      }

    /* It's necessary to read the volume label block */
    lif_read_block(input_file,0,data);
    /* Check it's a LIF disk */
    if(get_lif_int(data,2)!=0x8000)
      {
        fprintf(stderr,"This is not a LIF disk!\n");
        return(RETURN_ERROR);
      }
    /* output label */
    if((*(data+2))!=' ')
      {
        /* There is a volume label */
        printf("Volume : ");
        for(i=2; i<8; i++)
          {
            putchar(*(data+i));
          }
        printf(" ");
       }

    /* If the time stamp month is non-zero, print the time stamp */
    if(*(data+37))
      {
         printf("( formatted : ");
         print_date(data+36);
         printf(" )");
      }
    printf("\n");

    /* Print volume size */
    tracks=get_lif_int(data+24,4);
    surfaces=get_lif_int(data+28,4);
    blocks=get_lif_int(data+32,4);
    totalsize= tracks*surfaces*blocks;
    printf("Tracks: %d Surfaces: %d Blocks per Track: %d",tracks,surfaces,blocks);
    if(totalsize==0 || blocks == 0x9a009a0) 
      {
         printf(".\nWarning the medium was not initialized properly!\n");
      } else {
         printf(" Total size: %d Blocks, %d Bytes\n",totalsize,totalsize*256);
      }
    
    /* Find the directory */
    dir_start=get_lif_int(data+8,4);
    dir_length=get_lif_int(data+16,4);

    if(block_flag)
      {
        /* looking for a particular block, is it in the directory ? */
        if((block_no>=dir_start)&&(block_no<dir_start+dir_length))
          {
            lifstat_print_block_no(block_no);
            printf(" : directory\n");
            return(RETURN_OK);
          }
      }
    else
      {
        /* If not looking for a particular block, print directory location */
        printf("Directory start : ");
        lifstat_print_block_no(dir_start);
        printf(" end : ");
        lifstat_print_block_no(dir_start+dir_length-1);
        printf("\n");
      }

    /* Scan the directory */
    found=0;
    dir_end=0;
    for(dir_block=0; (dir_block<dir_length)&&(!dir_end)&&(!found); dir_block++)
      {
        lif_read_block(input_file,dir_block+dir_start,data);
        for(dir_entry=0; dir_entry<8; dir_entry++)
          {
            file_type=get_lif_int(data+(dir_entry<<5)+10,2);
            if(file_type==0) { continue; } /* skip deleted files */
            if(file_type==0xffff)
              {
                /* End of directory */
                dir_end=1;
                break;
              }
            file_start=get_lif_int(data+(dir_entry<<5)+12,4);
            file_length=get_lif_int(data+(dir_entry<<5)+16,4);
            /* update last used block */
            last_block=file_start+file_length-1;
            /* If searching for a particular block, is it in this file ? */
            if(block_flag && (block_no>=file_start) && 
               (block_no<file_start+file_length))
              {
                found=1;
                lifstat_print_block_no(block_no);
                printf(" : ");
                for(i=0; i<10; i++)
                  {
                    if((c=*(data+(dir_entry<<5)+i))==' ') { break; }
                    putchar(c);
                  }
                printf("\n");
              }
          }
      }

    if(block_flag && !found)
      {
        /* If the block wasn't part of a file, it's unused */
        lifstat_print_block_no(block_no);
        printf(" : unused\n");
      }

    if(!block_flag)
      {
        printf("User data start : ");
        lifstat_print_block_no(dir_start+dir_length);
        printf(" end : ");
        lifstat_print_block_no(last_block);
        printf("\n");
      }
    return(RETURN_OK);
  }

void lifstat_usage(void)
  {
    fprintf(stderr,"Usage : lifutils lifstat [-p] LIFIMAGEFILE\n");
    fprintf(stderr,"        Display the properties of a LIF image file\n");
    fprintf(stderr,"        or\n");
    fprintf(stderr,"        lifstat LIFIMAGEFILE BLOCKNUMBER\n");
    fprintf(stderr,"        or\n");
    fprintf(stderr,"        lifstat  [-p] LIFIMAGEFILE CYLINDER HEAD SECTOR \n");
    fprintf(stderr,"        Display filename located at BLOCKNUMER or CYLINDER/HEAD/SECTOR\n");
    fprintf(stderr,"        -p Print summary of LIF file system stats on a floppy disk.\n");
    fprintf(stderr,"           Note: this option is only supported on LINUX.\n");
    fprintf(stderr,"           Specify the floppy device instead of the lif-image-filename.\n");
    fprintf(stderr,"           See the LIFUTILS tutorial for details.\n");
    fprintf(stderr,"\n");
  }

int lifstat(int argc, char **argv)
  {
    int option; /* command line option character */
    int input_device; /* input file or device desciptor */
    int physical_flag; /* Option to use a physical device */
    int ret;

    unsigned int block_no; /* desired block number */
    unsigned int cylinder,head,sector; /* desired disk address */

    optind=1;
    physical_flag=0;
    /* Decode command line options */
    while((option=getopt(argc,argv,"p?"))!=-1)
      {
        switch(option)
          {
            case 'p' : physical_flag=1;
                       break;

            case '?' : lifstat_usage();
                       return(RETURN_OK);
          }
      }

    /* Is there a file name ? */
    if(optind==argc)
      {
        /* No, give error */
        lifstat_usage();
        return(RETURN_ERROR);
      }

    /* Open input device */
    if((input_device=lif_open(argv[optind],O_RDONLY | O_BINARY,0,physical_flag))==-1)
      {
        fprintf(stderr,"Error opening %s\n",argv[optind]);
        return(RETURN_ERROR);
      }

    /* process other command line arguments */
    optind++;
    switch(argc-optind)
      {
        case 0 : ret=lifstat_lif_status(input_device,0,0);
                 break;
        case 1 : block_no=atoi(argv[optind]);
                 if(block_no>=TOTAL_BLOCKS)
                   {
                     fprintf(stderr,"Block number out of range\n");
                     return(RETURN_ERROR);
                   }
                 ret=lifstat_lif_status(input_device,1,block_no);
                 break;
        case 3 : cylinder = atoi(argv[optind++]);
                 head = atoi(argv[optind++]);
                 sector = atoi(argv[optind])-1;
                 if(cylinder>=CYLINDERS)
                   {
                     fprintf(stderr,"Cylinder out of range\n");
                     return(RETURN_ERROR);
                   }
                 if(head>=HEADS)
                   {
                     fprintf(stderr,"Head out of range\n");
                     return(RETURN_ERROR);
                   }
                 if(sector>=SPT)
                   {
                     fprintf(stderr,"Sector out of range\n");
                     return(RETURN_ERROR);
                   }
                 block_no=cylinder*HEADS*SPT + head*SPT + sector;
                 ret=lifstat_lif_status(input_device,1,block_no);
                 break;
        default : lifstat_usage();
                  return(RETURN_ERROR);

      }
    lif_close(input_device);
    return(ret);
  }

