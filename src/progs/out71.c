/* out71.c -- A filter to add a directory entry to the start of a file
   (and pad the file so that it consists of 256 byte blocks) for sending
   directly to an HP71B via a suitable interface */
/* 2000 A. R. Duell, and placed under the GPL */

/* The HP71 can copy files from any HPIL device, not just mass-storage 
   devices. The data it expects to receive from a non-mass-storage device
   (like an interface) consists of :
   
   A 32 byte directory entry. This is identical to the 32 byte entry
   stored in the directory of a LIF volume, except that the starting
   block is set to 0

   The file data, padded (by this program with nulls) to a multiple of
   256 bytes.

   This program creates a suitable directory entry for the file read 
   from standard input  , and then sends it, together with the file data
   to standard output. 

   Further details can be found in the HP71 HPIL IDS */

/* One other problem is that some HP71 file types have their length in 
   nybbles stored in the type-specific bytes of the directory entry.
   Since PC files are stored in bytes, the number of nybbles in a PC
   file is always an even number. In the case of an HP71 file stored on 
   the PC  that originally contained an odd number of nybbles, this 
   program will normally send a file that appears to contain one more
   nybble -- the second half of the last byte of the PC file.

   If the -o command line flag is given, however, this program
   decreases the nybble count in the generated directory entry by
   1 to correct for this problem.

   Yes, it's a kludge, but there's no other way to do it that works
   for arbitrary PC files. */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"
#include "lif_create_entry.h"
#include "lif_const.h"

/* buffer to hold file */
unsigned char buffer[MAX_LENGTH+BLOCK_SIZE];

unsigned int read_file(void)
/* Read in the file from standard input */
  {
    int byte_counter; /* index of bytes read */
    int clear_counter; /* counter used for clearing one more block */
    int c; /* next character from standard input */
    byte_counter=0;
    /* Read in the file data */
    while(((c=getchar())!=EOF) && (byte_counter<MAX_LENGTH))
      {
        buffer[byte_counter]=c;
        byte_counter++;
      } 
    /* Is the file too large ? */
    if(byte_counter>=MAX_LENGTH)
      {
        fprintf(stderr,"Input file too large for an HP71\n");
        exit(1);
      }
    /* Clear one more block of the buffer */
    for(clear_counter=0; clear_counter<BLOCK_SIZE; clear_counter++)
      {
        buffer[byte_counter+clear_counter]=0;
      }
    return(byte_counter); /* return number of bytes read */
  }

void usage(void)
  {
    fprintf(stderr,"Usage : out71 [-o] filetype\n");
    exit(1);
  }

int main(int argc, char **argv)
  {
    int filetype; /* 2 byte file type code */
    unsigned int bytes_read; /* Size of input file */
    unsigned int blocks; /* Number of data blocks in file */
    unsigned char dir_entry[ENTRY_SIZE]; /* Directory entry */
    int odd_flag; /* Set if HP71 file contains odd number of nybbles */
    int option;

    SETMODE_STDIN_BINARY;
    SETMODE_STDOUT_BINARY;

    odd_flag=0;
    /* decode command line flags */
    optind=1;
    while((option=getopt(argc,argv,"o?"))!=-1)
      {
        switch(option)
          {
            case 'o' : odd_flag=1;
                       break;
            case '?' : usage();
          }
      }
    /* decode the file type string given on the command line */
    if(optind != argc-1)
      {
        fprintf(stderr,"out71 : Must give the file type\n");
        usage();
      }
    filetype=get_filetype(argv[optind]);
    if(filetype==-1)
      {
        fprintf(stderr,"out71 : Unknown file type : %s\n",argv[optind]);
        usage();
      }

    /* Read in input file */;
    bytes_read=read_file();

    /* Create directory entry */
    create_entry(dir_entry,"PCFILE",filetype,0,bytes_read, odd_flag);
    /* and output it */
    fwrite(dir_entry,sizeof(unsigned char), ENTRY_SIZE, stdout);
    
    /* Find number of blocks to output */
    blocks=(bytes_read+(BLOCK_SIZE-1))/BLOCK_SIZE;
    /* and output them */
    fwrite(buffer,sizeof(unsigned char),blocks*BLOCK_SIZE,stdout);
    exit(0);
  }

