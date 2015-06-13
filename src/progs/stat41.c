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

#include<stdio.h>
#include<unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include"descramble_41.h"
#include"print_41_data.h"

/* length of one file record */
#define RECORD_LEN 8

void read_record(unsigned char *record)
  {
    /* read one 8 byte record from standard input. Quit the program if EOF
       occurs */
    if(fread(record,sizeof(char),RECORD_LEN,stdin)!=RECORD_LEN)
      {
        /* Didn't read enough characters, so quit */
        exit(1);
      }
  }

void print_stack(int bldspec_flag, int flags_flag)
  {
    int i; /* register counter */
    char names[]={'T','Z','Y','X','L'}; /* Register names */
    unsigned char data[RECORD_LEN]; /* stack register data */
    for(i=0; i<5; i++)
      {
        /* read next stack register */
        read_record(data);
        /* print its name */
        printf(" %c : ",names[i]);
        /* print the register */
        print_record(data,0,bldspec_flag,flags_flag);
      }
  }

void print_alpha_reg(void)
  {
    /* print the alpha register, SIZE and statistics register start */
    int i; /* general counter */
    int skipping; /* skipping over nulls? */
    int size; /* HP41 SIZE */
    int sigma; /* Statistics register start */
    unsigned char data[RECORD_LEN]; /* one file record */
    unsigned char alpha[28]; /* alpha register */
    /* Read in 4 records, pack them into the alpha register */
    for(i=0;i<4;i++)
      {
        read_record(data);
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
  }

void print_44_flags(int verbose)
  {
    /* Print the first 44 system flags */
    int i; /* general counter */
    unsigned char data[RECORD_LEN]; /* file record */
    unsigned char flag_reg[7]; /* descrambled flag register */
    /* Read in the flags */
    read_record(data);
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
  }

void usage(void)
  {
    fprintf(stderr,"usage : stat41 [-b] [-f] [-v]\n");
    fprintf(stderr,"        -b : print strings on stack as BLDSPEC values\n");
    fprintf(stderr,"        -f : display strings on stack as flag settings\n");
    fprintf(stderr,"             (if appropriate)\n");
    fprintf(stderr,"        -v : Display user flags verbosely\n");
    exit(1);
  }

int main(int argc, char **argv)
  {
    int option; /* current option character */
    int bldspec_flag=0; /* display BLDSPEC values? */
    int flags_flag=0; /* display RCLFLAG values? */
    int verbose=0; /* print user flags one to a line */

#ifdef _WIN32
    setmode(fileno(stdin), O_BINARY);
#endif

    /* Process command line options */
    optind=1;
    while((option=getopt(argc,argv,"bfv?"))!=-1)
      {
        switch(option)
          {
            case 'b' : bldspec_flag=1;
                       break;
            case 'f' : flags_flag=1;
                       break;
            case 'v' : verbose=1;
                       break;
            case '?' : usage();
          }
      }
    if(optind!=argc)
      {
        /* There must not be any other arguments */
        usage();
      }
    /* Print the RPN stack */
    printf("RPN Stack : \n\n");
    print_stack(bldspec_flag,flags_flag);
    printf("\n");
    /* Print the alpha register etc */
    print_alpha_reg();
    printf("\n");
    /* Print the flags */
    print_44_flags(verbose);
    exit(0);
  }

