/* lifget.c -- Get a file from a LIF image file */
/* 2000 A. R. Duell, and placed under the GPL */

#include<stdio.h>
#include<fcntl.h>
#include <stdlib.h>
#include "config.h"
#include "lifutils.h"
#include"lif_block.h"
#include"lif_dir_utils.h"
#include "lif_const.h"



void lifget_usage(void)
  {
    fprintf(stderr,"Usage : lifutils lifget [-r] [-b] [-l] [-p] LIFIMAGEFILE LIFFILENAME OUTPUTFILE\n");
    fprintf(stderr,"        or\n");
    fprintf(stderr,"        lifutils lifget [-r] [-b] [-l] [-p] LIFIMAGEFILE LIFFILENAME > output file\n");
    fprintf(stderr,"        Extract a file from a LIF image file\n");
    fprintf(stderr,"        -r flag to remove directory entry on start of file \n"); 
    fprintf(stderr,"        -b Flag to copy the blocks used by the file,\n");
    fprintf(stderr,"            ignoring the file length information\n");
    fprintf(stderr,"        -l Relaxe file name checking, allow underscores in file names\n");
    fprintf(stderr,"        -p Read file from a floppy disk with a LIF file system.\n");
    fprintf(stderr,"           Note: this option is only supported on LINUX.\n");
    fprintf(stderr,"           Specify the floppy device instead of the lif-image-filename.\n");
    fprintf(stderr,"           See the LIFUTILS tutorial for details.\n");
    fprintf(stderr,"\n");
  }

void lifget_file_copy(int input_device, FILE* output_file, 
               int start, int length)
  {
    /* Copy length bytes starting at block start from LIF input_device
       to output_file */
  
    unsigned int complete_blocks; /* Number of complete blocks to copy */
    unsigned int leftover_bytes; /* And the odd bytes on the end */
    unsigned int block; /* Current block offset from start */
    unsigned char data[SECTOR_SIZE];

    complete_blocks=length/SECTOR_SIZE;
    if(complete_blocks!=0)
      {
        /* Copy the complete blocks first */
        for(block=0; block<complete_blocks; block++)
          {
            lif_read_block(input_device,start+block,data);
            fwrite(data,sizeof(char),SECTOR_SIZE,output_file);
          }
      }
    leftover_bytes=length%SECTOR_SIZE;
    if(leftover_bytes!=0)
      {
        /* Odd bytes on the end -- read one more block */
        lif_read_block(input_device,start+complete_blocks,data);
        fwrite(data,sizeof(char),leftover_bytes,output_file);
      }
  }

int lifget(int argc, char **argv)
  {
    /* System variables */
    int option; /* Command line option character */
    int remove_dir_flag; /* Remove directory entry on start of output */
    int block_flag; /* Copy blocks */
    int physical_flag; /* Option to use a physical device */
    int lax; /* Option to relax file name checking */

    int input_device; /* Descriptor of input device */
    FILE *output_file; /* Output file stream */
    char cmp_name[10]; /* File name to look for */
    
    /* LIF disk values */
    unsigned int dir_start; /* first block of the directory */
    unsigned int dir_length; /* length of directory in blocks */

    /* Directory search values */
    unsigned int dir_end; /* Set at end of directory */
    unsigned int found_file; /* Set when file found */
    unsigned int dir_entry; /* Directory entry within current block */
    unsigned int dir_block; /* Current block offset from start of directory */
    unsigned int file_type; /* file type word */
    unsigned char dir_data[SECTOR_SIZE]; /* Current directory block data */

    /* File values */
    int file_start; /* Starting block number of the file to copy */
    int file_len; /* Length of file in bytes */

    /* Process command line options */
    remove_dir_flag=0;
    block_flag=0;
    physical_flag=0;
    lax=0;

    optind=1;
    while ((option=getopt(argc,argv,"prbl?"))!=-1)
      {
        switch(option)
          {
            case 'r' : remove_dir_flag=1;
                       break;
            case 'b' : block_flag=1;
                       break;
            case 'p' : physical_flag=1;
                       break;
            case 'l' : lax=1;
                       break;
            case '?' : lifget_usage();
                       return(RETURN_OK);
          }
      }

    /* Are the right number of names specified ? */
    if( (optind != argc-2) && (optind != argc-3) )
      {
        /* No, give an error */
        lifget_usage();
        return(RETURN_ERROR);
      }

    /* Check file name */
    if(check_filename(argv[optind+1],lax)==0)
      {
        fprintf(stderr,"Illegal LIF file name\n");
        return(RETURN_ERROR);
      }


    /* Open input device */
    if((input_device=lif_open(argv[optind],O_RDONLY | O_BINARY,0,physical_flag))==-1)
      {
        fprintf(stderr,"Error opening %s\n",argv[optind]);
        return(RETURN_ERROR);
      }


    /* Now read block 0 to find where the directory is */
    lif_read_block(input_device,0,dir_data);

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

    /* Scan the directory */
    dir_end=0;
    found_file=0;
    for(dir_block=0; dir_block<dir_length; dir_block++)
      {
        lif_read_block(input_device,dir_block+dir_start,dir_data);
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

    /* Open the output file */
    if(optind==argc-3)
      {
        output_file=fopen(argv[argc-1],"wb");
      }
    else
      {
        SETMODE_STDOUT_BINARY ;
        output_file=stdout;
      }

    /* If the -d flag was specified, send the directory entry */
    if(! remove_dir_flag)
      {
        fwrite(dir_data+(dir_entry<<5),sizeof(char),32,output_file);
      }

    /* Find the file start and length */
    file_start=get_lif_int(dir_data+(dir_entry<<5)+12,4);
    if(block_flag)
      {
        file_len=get_lif_int(dir_data+(dir_entry<<5)+16,4)*256;
      }
    else
      {
        file_len=file_length(dir_data+(dir_entry<<5),NULL);
      }

    /* Actually copy the file */ 
    lifget_file_copy(input_device,output_file,file_start,file_len);

    /* tidy up and quit */
    if(optind==argc-3)
      {
        fclose(output_file);
      }
    lif_close(input_device);
    return(RETURN_OK);      
  }
