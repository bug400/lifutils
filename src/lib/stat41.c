/* stat41.c -- a filter to display an HP41 status file */
/* 2000 A. R. Duell, and placed under the GPL */

/* An HP41 status file consits of 10 8-byte records. The first 5 of these 
   contain the contents of the RPN stack in the order T, Z, Y, X, L. These
   values are encoded in the same way as the records of an sdata file.

   The next 4 records, once descrambled into 7 byte values give the  
   contents of the alpha register (the first record is the rightmost part of
   alpha) as the last 24 bytes and the SIZE and location of the statistical
   registers encoded into the top 6 nybbles of the last record.

   Finally, the 10th record contains the settings of the first 44 user
   flags in the first 44 bits. */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"
#include "lifutils.h"
#include "lif_dir_utils.h"
#include"descramble_41.h"
#include"print_41_data.h"


int stat41_read_record(FILE *fp, unsigned char *record)
  {
    /* read one 8 byte record from standard input. Quit the program if EOF
       occurs */
    if(fread(record,sizeof(char),SDATA_RECORD_SIZE,fp)!=SDATA_RECORD_SIZE)
      {
        /* Didn't read enough characters, so quit */
        return(RETURN_ERROR);
      }
    return(RETURN_OK);
  }

int stat41_print_stack(FILE *fp,int bldspec_flag, int flags_flag)
  {
    int i; /* register counter */
    char names[]={'T','Z','Y','X','L'}; /* Register names */
    unsigned char data[SDATA_RECORD_SIZE]; /* stack register data */
    for(i=0; i<5; i++)
      {
        /* read next stack register */
        if(stat41_read_record(fp,data)!=RETURN_OK)return(RETURN_ERROR);
        /* print its name */
        printf(" %c : ",names[i]);
        /* print the register */
        print_record(data,0,bldspec_flag,flags_flag);
      }
    return(RETURN_OK);
  }

int stat41_print_alpha_reg(FILE *fp)
  {
    /* print the alpha register, SIZE and statistics register start */
    int i; /* general counter */
    int skipping; /* skipping over nulls? */
    int size; /* HP41 SIZE */
    int sigma; /* Statistics register start */
    unsigned char data[SDATA_RECORD_SIZE]; /* one file record */
    unsigned char alpha[28]; /* alpha register */
    /* Read in 4 records, pack them into the alpha register */
    for(i=0;i<4;i++)
      {
        if(stat41_read_record(fp,data)!=RETURN_OK)return(RETURN_ERROR);
        descramble(data,alpha+(21-7*i));
      }
    /* print the alpha register */
    skipping=1;
    printf("Alpha : \"");
    for(i=4;i<28;i++)
      {
        /* Have there been any printable characters ? */
        skipping=alpha[i]?0:skipping;
        if(!skipping)
          {
            print_char(alpha[i]);
          }
      }
    printf("\"\n\n");
    /* calculate size and statistics pointer */
    size=(alpha[0]<<4)+(alpha[1]>>4);
    sigma=((alpha[1]&0xf)<<8)+alpha[2];
    printf("SIZE = %03d\nStatistics start = %03d\n",size,sigma);
    return(RETURN_OK);
  }

int stat41_print_44_flags(FILE *fp,int verbose)
  {
    /* Print the first 44 system flags */
    int i; /* general counter */
    unsigned char data[SDATA_RECORD_SIZE]; /* file record */
    unsigned char flag_reg[7]; /* descrambled flag register */
    /* Read in the flags */
    if(stat41_read_record(fp,data)!=RETURN_OK)return(RETURN_ERROR);
    descramble(data,flag_reg); 
    /* print the first 44 flags */
    if(verbose)
      {
        printf(" User Flags : \n");
        for(i=0;i<44;i++)
          {
            printf(" Flag %02d : %s\n",i,
                   (flag_reg[i>>3]&(1<<(7-(i&7))))?"Set":"Clear");
          }
      }
    else
      {
        printf("User Flags 00--43 : ");
        for(i=0;i<5;i++)
          {
            print_binary(flag_reg[i],8);
          }
         print_binary(flag_reg[5]>>4,4);
         printf("\n");
      }
    return(RETURN_OK);
  }

void stat41_usage(void)
  {
    fprintf(stderr,"Usage : lifutils stat41 [-b] [-f] [-v] [-r] INPUTFILE [> output file]\n");
    fprintf(stderr,"        or\n");
    fprintf(stderr,"        lifutils stat41 [-b] [-f] [-v] [-r] < input file [> output file]\n");
    fprintf(stderr,"        Display information of a raw HP41 status file\n");
    fprintf(stderr,"        -b : print strings on stack as BLDSPEC values\n");
    fprintf(stderr,"        -f : display strings on stack as flag settings\n");
    fprintf(stderr,"             (if appropriate)\n");
    fprintf(stderr,"        -v : Display user flags verbosely\n");
    fprintf(stderr,"        -r skip an existing LIF header of the input file\n");
    fprintf(stderr,"\n");
  }

int stat41(int argc, char **argv)
  {
    int option; /* current option character */
    int bldspec_flag=0; /* display BLDSPEC values? */
    int flags_flag=0; /* display RCLFLAG values? */
    int verbose=0; /* print user flags one to a line */
    int skip_lif=0;
    FILE *fp;


    /* Process command line options */
    optind=1;
    while((option=getopt(argc,argv,"bfrv?"))!=-1)
      {
        switch(option)
          {
            case 'b' : bldspec_flag=1;
                       break;
            case 'f' : flags_flag=1;
                       break;
            case 'v' : verbose=1;
                       break;
            case 'r' : skip_lif=1;
                       break;
            case '?' : stat41_usage();
                       return(RETURN_OK);
          }
      }
    if((optind!=argc) && (optind!= argc-1))
       {
         stat41_usage();
         return(RETURN_ERROR);
       }
    if(optind== argc-1) {
        fp=fopen(argv[argc-1],"rb");
     } else {
        fp=stdin;
        SETMODE_STDIN_BINARY;
     }

    if (fp == NULL) {
       fprintf(stderr,"Error: cannot open input file\n");
       return(RETURN_ERROR);
     }

    if(skip_lif) {
       if(skip_lif_header(fp,"STAT41")) {
          fclose(fp);
          fprintf(stderr,"Error: not a LIF HP-41 status file\n");
          return(RETURN_ERROR);
       } 
    } 

    /* Print the RPN stack */
    printf("RPN Stack : \n\n");
    stat41_print_stack(fp,bldspec_flag,flags_flag);
    printf("\n");
    /* Print the alpha register etc */
    stat41_print_alpha_reg(fp);
    printf("\n");
    /* Print the flags */
    stat41_print_44_flags(fp,verbose);
    if(fp!=stdin) fclose(fp);
    return(RETURN_OK);
  }

