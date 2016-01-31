/* lifheader.c -- display header information of a LIF file */
/* 2016 J. Siebold, and placed under the GPL */

#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include"lif_block.h"
#include"lif_dir_utils.h"
#include "lif_create_entry.h"
#include "lif_const.h"
#ifndef _WIN32
#define O_BINARY 0
#endif


#define FALSE 0
#define TRUE 1
#define DEBUG 0
#define debug_print(fmt, ...) \
   do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

#define filelength_in_blocks(i) \
   i/SECTOR_SIZE + (int) ((i % SECTOR_SIZE) != 0) 

/* offsets for the parts of a timestamp */
#define YEAR_OFF 0
#define MONTH_OFF 1
#define DAY_OFF 2
#define HOUR_OFF 3
#define MINUTE_OFF 4
#define SECOND_OFF 5

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
       

void usage(void)
{
   fprintf(stderr,
   "Usage: lifheader filename\n");
   fprintf(stderr,"      if filename is omitted, input comes from standard input\n");
   fprintf(stderr,"\n");
   exit(1);
}


int main(int argc, char **argv)
  {
    
    /* file header values */
    unsigned char file_name[NAME_LEN]; /* File name  */
    unsigned char dir_entry[ENTRY_SIZE]; /* Directory Entry of new file */
    int file_type;
    int i;
    char typestring[10];
    int data_length; /* length of input file in bytes without header */
    int num_blocks; /* size of input file in blocks */
    FILE* input_file; /* input file or stdin */

    
    /* Process command line */
    if(argc != 1 && argc !=2) {
       usage();
       exit(1);
    }
    /* open input file, if none specified use standard input */
    if ( argc ==2) {
       input_file= fopen(argv[1],"rb");
       if (input_file == (FILE *) NULL ) {
          fprintf(stderr,"can't open File %s\n",argv[1]);
          exit(2);
       }
    }
    else {
#ifdef _WIN32
       setmode(fileno(stdin), O_BINARY);
#endif
       input_file= stdin;
    }

    /* read lif entry from input file header */
    fread(dir_entry,sizeof(unsigned char),ENTRY_SIZE,input_file);
    data_length= file_length(dir_entry,typestring);
    num_blocks= filelength_in_blocks(data_length);
    file_type= get_lif_int(&dir_entry[10],2);

    /* get file name */
    for (i=0; i<NAME_LEN; i++) 
       file_name[i]= dir_entry[i];

     
    printf("File name       : ");
    for (i=0; i<NAME_LEN; i++) printf("%c",file_name[i]);
    printf("%s","\n");

    printf("File type       : %x ",file_type);
    if(typestring[0]== '?') {
          printf("(unknown file type)\n");
    } else {
          printf("(%s) \n",typestring);
    }
    printf("Data length     : %d\n",data_length);
    printf("Number of blocks: %d\n",num_blocks);
    if(*(dir_entry+21))
         {
           printf("Creation date   : ");
           /* If the month is not 0, it's a valid date */
           print_date(dir_entry+20);
           printf("\n");
         }
    exit(0);      
  }



