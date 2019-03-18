/* liflabel.c -- label a lif image file */
/* 2014, J. Siebold, and placed under the GPL */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "lif_block.h"
#include "lif_dir_utils.h"
#include "lif_const.h"

#define DEBUG 0
#define debug_print(fmt, ...) \
   do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)


void usage(void)
  {
    fprintf(stderr,
    "Usage: liflabel [-c] [-p] lif-image-filename label\n");
    fprintf(stderr,"      if label is omitted, the current label is displayed\n");
    fprintf(stderr,"\n");
    fprintf(stderr,
   "      -c clear an existing label. The label argument must be omitted\n");
    fprintf(stderr,"      -p Label a LIF file system on a floppy disk.\n");
    fprintf(stderr,"         Note: this option is only supported on LINUX.\n");
    fprintf(stderr,"         Specify the floppy device instead of the lif-image-filename.\n");
    fprintf(stderr,"         See the LIFUTILS tutorial for details.\n");
    exit(1);
  }

int main(int argc, char **argv)
  {
    /* System variables */
    int option; /* Command line option character */
    int clear_flag; /* Clear an existing label */
    int physical_flag; /* Option to use a physical device */
    int lif_device; /* Descriptor of input device */
    char new_label[LABEL_LEN]; /* New label name */
    int i;
    
    /* LIF disk values */
    unsigned char dir_data[SECTOR_SIZE]; /* sector 0 */

    /* Process command line options */
    clear_flag=0;
    physical_flag=0;
    optind=1;
    while ((option=getopt(argc,argv,"pc?"))!=-1)
      {
        switch(option)
          {
            case 'c' : clear_flag=1;
                       break;
            case 'p' : physical_flag=1;
                       break;
            case '?' : usage();
          }
      }

    /* Are the right number of names specified ? */
    if( (optind != argc-1  ) && (optind != argc-2))
      {
        /* No, give an error */
        usage();
      }

    /* Open lif device */
    if((lif_device=lif_open(argv[optind],O_RDWR | O_BINARY,0,physical_flag))==-1)
      {
        fprintf(stderr,"Error opening %s\n",argv[optind]);
        exit(1);
      }
 
    if (optind == argc -2 )  
      {
       if (clear_flag) usage();
       if(check_labelname(argv[optind+1])==0) {
           fprintf(stderr,"Illegal label name\n");
           exit(1);
       }
       pad_label(argv[optind+1],new_label);
      }
    else
       new_label[0]='\0';


    /* Now read block 0 to find where the directory is */
    lif_read_block(lif_device,0,dir_data);

    /* Make sure it's a LIF disk */
    if(get_lif_int(dir_data+0,2)!=0x8000)
      {
        fprintf(stderr,"This is not a LIF disk!\n");
        exit(1);
      }

     /* Output label */
     if((*(dir_data+2))!=' ')
      {
        /* There is a volume label */
        printf("Old Volume : ");
        for(i=2; i<8; i++)
          {
            putchar(*(dir_data+i));
          }
        printf("\n");
       }

     /* Clear Label */
     if (clear_flag) {
        for(i=2; i<8; i++) *(dir_data+i)=' ';
        lif_write_block(lif_device,0,dir_data);
     }

     /* new label specified; apply it */
     if (new_label[0] != '\0') {
        for(i=2; i<8; i++)
          {
            *(dir_data+i)= new_label[i-2];
          }
        printf("New Volume : ");
        for(i=2; i<8; i++)
          {
            putchar(*(dir_data+i));
          }
        printf("\n");
        lif_write_block(lif_device,0,dir_data);
     }
    /* tidy up and quit */
    lif_close(lif_device);
    exit(0);      
  }
