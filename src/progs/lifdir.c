/* lifdir.c -- Display the directory of a LIF disk */
/* 2000 A. R. Duell, and placed under the GPL */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"
#include "lif_block.h"
#include "lif_dir_utils.h"
#include "lif_const.h"

/* offsets for the parts of a timestamp */
#define YEAR_OFF 0
#define MONTH_OFF 1
#define DAY_OFF 2
#define HOUR_OFF 3
#define MINUTE_OFF 4
#define SECOND_OFF 5

/* csv separator */
#define SEP ','

void print_date(unsigned char *date)
  {
    /* Print an HP-style date and time stamp. On entry, date points to
       the first byte of a 6-byte time stamp */
    printf("%02d/%02d/%02d %02d:%02d:%02d",
         bcd_to_dec(*(date+DAY_OFF)),
         bcd_to_dec(*(date+MONTH_OFF)),
         bcd_to_dec(*(date+YEAR_OFF)),
         bcd_to_dec(*(date+HOUR_OFF)),
         bcd_to_dec(*(date+MINUTE_OFF)),
         bcd_to_dec(*(date+SECOND_OFF)));
  }
       
void print_dir_entry(unsigned char *entry, int verbosity)
  {
    /* Decode and print a directory entry. entry points to a 32 byte 
       directory entry, as described in appendix D of the HP71 HPIL
       owner's manual */
    int i; /* general counter */
    int length; /* File length from directory */
    int total_blocks; /* Number of blocks occupied by file */
    int start_block; /* file start block */
    unsigned int implementation_byte; /* implementation byte */

    char file_type[10]; /* storage for the file type string */

    /* Print the filename */
    for(i=0; i<10; i++)
      {
        putchar(*(entry+i));
      }
    /* default directory listing */
    if(verbosity > 0)
      {
       /* find and print the file type */
       length=file_length(entry,file_type);
       printf("  %-10s  ",file_type);
       total_blocks=get_lif_int(entry+16,4);
       /* Print the length, both from the directory, and from the number of 
          blocks */
       printf("%5d/%-5d    ",length,total_blocks*256);
       /* Print the file time and date */
       if(*(entry+21))
         {
           /* If the month is not 0, it's a valid date */
           print_date(entry+20);
         }
       else printf("                 ");
      }
    /* output additional information */
    if (verbosity > 1)
      {
       start_block=get_lif_int(entry+12,4);
       printf("%5d %5d ",start_block, total_blocks);
       for(i=0; i<6; i++)
         {
            implementation_byte=(unsigned char) get_lif_int(entry+26+i,1);
            printf("%02X",implementation_byte);
         }
            
      }
    
    printf("\n");
  } 

void csv_dir_entry(unsigned char *entry)
  {
    /* Decode a directory entry and output it as csv. entry points to a 32 
       byte directory entry, as described in appendix D of the HP71 HPIL
       owner's manual */
    int i; /* general counter */
    int length; /* File length from directory */
    char file_type[10]; /* storage for the file type string */

    /* output the filename */
    for(i=0; i<10; i++)
      {
	if( *(entry+i) == ' ') break;
        putchar(*(entry+i));
      }
    printf("%c",SEP);

    /* output the file type */
    length=file_length(entry,file_type);
    printf("%s",file_type);
    printf("%c",SEP);

    /* output the file type in hex */
    printf("%d", get_lif_int(entry+10,2));
    printf("%c",SEP);

    /* output the file length */
    printf("%d",length);
    printf("%c",SEP);

    /* output the start block */
    printf("%d",get_lif_int(entry+12,4));
    printf("%c",SEP);

    /* output the number of blocks */
    printf("%d",get_lif_int(entry+16,4));

    /* output date and time */
    for (i=0; i<6; i++) 
      {
	 printf("%c",SEP);
         printf("%d",bcd_to_dec(*(entry+20+i)));
      }

    /* output the impelentation bytes */
    for(i=0; i<6; i++)
      {
         printf("%c",SEP);
         printf("%d",get_lif_int(entry+26+i,1));
      }
    printf("\n");
  } 

void usage(void)
  {
     fprintf(stderr,"Usage:lifdir [-n] [-v l] <lif-image-file>\n");
     fprintf(stderr,"\n");
     fprintf(stderr,"      -n flag to display file names only\n");
     fprintf(stderr,"      -v l verbosity level\n");
     fprintf(stderr,"         0: display file names only (same as -n)\n");
     fprintf(stderr,"         1: default directory listing\n");
     fprintf(stderr,"         2: add file start block, file length in blocks and \n");
     fprintf(stderr,"              implementation bytes to directory listing\n");
     fprintf(stderr,"      -c output all directory information as csv,\n");
     fprintf(stderr,"         see program documentation for details.\n");
     fprintf(stderr,"\n");
     fprintf(stderr,"         Note: verbosity level and csv output options are\n");
     fprintf(stderr,"         mutually exclusive.\n");
     exit(1);
  }

int main(int argc, char **argv)
  {
    /* system variables */
    int option; /* Command line option character */
    int input_device; /* Input file or device descriptor */
    int verbosity;    /* extent of information */
    int csv_flag;     /* csv output */
    int physical_flag; /* pyhsical disk access flag */
    char *snum_verbosity= (char *) NULL; /* arg to -v option */
    int i; /* General index counter */
    unsigned char data[SECTOR_SIZE]; /* buffer to hold current block */

    /* LIF disk values */
    unsigned int dir_start; /* first block of directory */
    unsigned int dir_length; /* Number of blocks in directory */
    unsigned int last_block; /* last used block on the disk */
    unsigned int surfaces;  /* no of disk surfaces in lif header */
    unsigned int tracks; /* no of tracks in lif header */
    unsigned int blocks; /* no of blocks in lif header */
    unsigned int totalsize; /* size of medium in blocks */
    unsigned int num_files; /* number of files on medium */


    /* Directory search values */
    unsigned int dir_end; /* Set at end of directory */
    unsigned int dir_entry; /* Directory entry within current block */
    unsigned int dir_block; /* Current block offset from start of directory */
    unsigned int file_type; /* File type word */

    /* file specific values */
    unsigned int file_start;
    unsigned int file_len;

    /* process command line options */
    optind=1;
    physical_flag=0;
    verbosity=-1;
    csv_flag=0;
    while((option=getopt(argc,argv,"v:npc?"))!=-1)
      {
        switch(option)
          {
             case 'n' : verbosity=0;
                        break;
             case 'v' : snum_verbosity=optarg;
                        break;
             case 'p' : physical_flag=1;
                        break;
             case 'c' : csv_flag=1;
                        break;
             case '?' : usage();
           }
       }

    /* Is one input device specified? */
    if(optind != argc-1)
      {
        /* if not, give error */
        usage();
      }
    /* get number of verbosity level */
    if ( snum_verbosity != (char *) NULL) {
       if (sscanf(snum_verbosity,"%d",&verbosity) !=1) {
          printf("Err2");
          usage();
       }
       if (verbosity < 0 || verbosity > 2) usage();
    }

    /* error if verbosity and csv specified */
    if (csv_flag == 1 && verbosity != -1) usage();

    /* set default verbodity level if not specified or csv */
    if (verbosity == -1) {
       if (csv_flag == 1) verbosity=0;
       else verbosity=1;
    }
    
    /* open input device */
    if((input_device=lif_open(argv[argc-1],O_RDONLY | O_BINARY,0,physical_flag))==-1)
      {
        fprintf(stderr,"Error opening %s\n",argv[argc-1]);
        exit(1);
      }

    /* Now read block 0, the volume label block */
    lif_read_block(input_device,0,data);

    /* Check that this is a LIF disk or image */
    if(get_lif_int(data+0,2)!=0x8000)
      {
        fprintf(stderr,"This is not a LIF disk!\n");
        exit(1);
      }
    if(verbosity > 0) 
      {
       printf("Volume : ");
       if((*(data+2))!=' ')
       {
           /* There is a volume label */
           for(i=2; i<8; i++)
             {
               putchar(*(data+i));
             }
           printf(" ");
          }

        /* If the time stamp month is non-zero, print the time stamp */
       if(*(data+37))
         {
           printf(", formatted : "); 
           print_date(data+36);
         }
       printf("\n");
      /* Print volume size */
      tracks=get_lif_int(data+24,4);
      surfaces=get_lif_int(data+28,4);
      blocks=get_lif_int(data+32,4);
      totalsize= tracks*surfaces*blocks;
      printf("Tracks: %d Surfaces: %d Blocks/Track: %d",tracks,surfaces,blocks);
      if(totalsize==0 || blocks == 0x9a009a0)
      {
         printf(".\nWarning the medium was not initialized properly!\n");
      } else {
         printf(" Total size: %d Blocks, %d Bytes\n",totalsize,totalsize*256);
      }
    }

    /* Find where the directory is */
    dir_start=get_lif_int((data+8),4);
    dir_length=get_lif_int((data+16),4);
    last_block=0;

    /* Now scan the directory */
    num_files=0;
    for(dir_block=0; dir_block<dir_length; dir_block++)
      {
         dir_end=0;
         lif_read_block(input_device,dir_block+dir_start,data);
         for(dir_entry=0; dir_entry<8; dir_entry++)
           {
             file_type=get_lif_int((data+(dir_entry<<5)+10),2);
             if(file_type==0) { continue; } /* Skip over deleted files */
             if(file_type==0xffff)
               {
                 /* This is the end of the directory */
                 dir_end=1;
                 break;
               }
	    if ( csv_flag) 
	      {
                 csv_dir_entry(data+(dir_entry<<5));
	      }
	    else
	      {
                 print_dir_entry(data+(dir_entry<<5),verbosity);
	      }
            file_start=get_lif_int(data+(dir_entry<<5)+12,4);
            file_len=get_lif_int(data+(dir_entry<<5)+16,4);
            /* update last used block */
            last_block=file_start+file_len-1;
            num_files++;
           }
         if(dir_end) { break; } /* Quit at end of directory */
      }
    lif_close(input_device);
    if(verbosity > 0) {
       printf("%d files (%d max), ",num_files,dir_length*8);
       printf("last block used: %d of %d\n",last_block,totalsize);
    }
    exit(0); 
  }

