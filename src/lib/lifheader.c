/* lifheader.c -- display header information of a LIF file */
/* 2016 J. Siebold, and placed under the GPL */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "lifutils.h"
#include "lif_block.h"
#include "lif_dir_utils.h"
#include "lif_create_entry.h"
#include "lif_const.h"

#define DEBUG 0

void lifheader_usage(void)
{
   fprintf(stderr,"Usage:  lifutils lifheader LIFFILENAME [> output file]\n");
   fprintf(stderr,"or:\n");
   fprintf(stderr,"        lifutils lifheader < input file [> output file]\n");
   fprintf(stderr,"        Show the header information of a LIF file LIFFILENAME\n");
   fprintf(stderr,"\n");
}


int lifheader(int argc, char **argv)
  {
    
    /* file header values */
    unsigned char file_name[NAME_LEN]; /* File name  */
    unsigned char dir_entry[ENTRY_SIZE]; /* Directory Entry of new file */
    unsigned char implementation_byte; /* Implementation byte */
    int file_type;
    int i;
    char typestring[10];
    int data_length; /* length of input file in bytes without header */
    int num_blocks; /* size of input file in blocks */
    FILE* fp; /* input file or stdin */

    int option;

    while ((option=getopt(argc,argv,"?"))!=-1)
      {
        switch(option)
          {
            case '?' : lifheader_usage();
                       return(RETURN_OK);
          }
      }

    
    /* Process command line */
    if(argc != 1 && argc !=2) {
       lifheader_usage();
       return(RETURN_ERROR);
    }
    /* open input file, if none specified use standard input */
    if ( argc ==2) {
       fp= fopen(argv[1],"rb");
       if (fp == (FILE *) NULL ) {
          fprintf(stderr,"can't open File %s\n",argv[1]);
          return(RETURN_ERROR);
       }
    }
    else {
       SETMODE_STDIN_BINARY;
       fp= stdin;
    }

    /* read lif entry from input file header */
    fread(dir_entry,sizeof(unsigned char),ENTRY_SIZE,fp);
    data_length= file_length(dir_entry,typestring);
    num_blocks= filelength_in_blocks(data_length);
    file_type= get_lif_int(&dir_entry[10],2);

    /* get file name */
    for (i=0; i<NAME_LEN; i++) 
       file_name[i]= dir_entry[i];

     
    printf("File name           : ");
    for (i=0; i<NAME_LEN; i++) printf("%c",file_name[i]);
    printf("%s","\n");

    printf("File type           : %x ",file_type);
    if(typestring[0]== '?') {
          printf("(unknown file type)\n");
    } else {
          printf("(%s) \n",typestring);
    }
    printf("Data length         : %d\n",data_length);
    printf("Number of blocks    : %d\n",num_blocks);
    if(*(dir_entry+21))
         {
           printf("Creation date       : ");
           /* If the month is not 0, it's a valid date */
           print_date(dir_entry+20);
           printf("\n");
         }
    printf("Implementation bytes: ");
    for(i=0; i<6; i++)
        {
           implementation_byte=(unsigned char) get_lif_int(dir_entry+26+i,1);
           printf("%02X",implementation_byte);
        }
    printf("\n");
    if(fp!=stdin)fclose(fp);
    return(RETURN_OK);      
  }
