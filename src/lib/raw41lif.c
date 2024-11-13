/* raw41lif.c -- a filter to process HP41 raw files  files */
/* 2014 Joachim Siebold and placed under the GPL */

/* raw41lif reads hp41 program raw files and creates a lif file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include "config.h"
#include "lifutils.h"
#include "lif_create_entry.h"
#include "lif_dir_utils.h"
#include "lif_const.h"

#define DEBUG 0

void raw41lif_usage(void)
{
   fprintf(stderr,"Usage : lifutils raw41lif LIFFILENAME INPUTFILE > output file \n");
   fprintf(stderr,"        or\n");
   fprintf(stderr,"        lifutils raw41lif LIFFILENAME < input file > output file \n");
   fprintf(stderr,"        Convert a HP41C program raw file to a LIF file\n");
   fprintf(stderr,"        -r skip an existing LIF header of the input file\n");
   fprintf(stderr,"\n");
}



void raw41lif_short_lbl(int *pc)
  {
    (*pc)++; /* inc program counter */
  }


void raw41lif_short_sto_rcl(int *pc)
  {
    (*pc)++; /* and pc */
  }


void raw41lif_short_gto(int *pc)
  {
    (*pc)+=2; /* inc program counter */
  }


void raw41lif_swap_lbl(int *pc)
  {
    (*pc)+=2; /* And pc */
  }


int raw41lif_global_end(unsigned char *memory, int *pc)
  {
    int end_flag;
    if(memory[(*pc)+2]&0x80)
      {
        /* Miss off the first character of the string (key asignment) */
        end_flag=0;
      }
    else
      {
        /* It's an END */
        /* Bit 5 of the third byte distinguishes a local end from a global
           one */
        end_flag=1;
      }
    (*pc)+=(memory[(*pc)+2]&0x80)?(memory[(*pc)+2]&0xf)+3:3;
    return(end_flag); /* Is this the end of the program? */
  }


void raw41lif_decode_gto_xeq(int *pc)
  {
    (*pc)+=3; /* This is a 3 byte instruction */
  }

void raw41lif_decode_alpha_gto(unsigned char *memory, int *pc)
  {
    /* 'Extend Your HP41' section 15.7 says that if the high nybble of 
       the second byte is not f, then the pc is only incremented by 2 */
    (*pc)+=((memory[(*pc)+1]>>4)==0xf)?(memory[(*pc)+1]&0xf)+2:2;
  }


void raw41lif_decode_digits(unsigned char *memory, int *pc)
  {
    /* This is the only function where the number of bytes can't be 
       deduced from the opcode/suffix. Instead, keep on printing until
       the next byte is not numeric */
    do
      {
        (*pc)++;
      }
    while((memory[*pc]>=0x10) && (memory[*pc]<=0x1c));
  }



void raw41lif_decode_1_byte(int *pc)
  {
    (*pc)++; /* Move the program counter */
  }



void raw41lif_decode_2_byte(int *pc)
  {
   (*pc)+=2; /* Move pc on 2 bytes */
  }


void raw41lif_decode_row_1(unsigned char *memory, int *pc)
  {
    if(memory[*pc]>=0x1d)
      {
        /* It's an alpha GTO/XEQ */
        raw41lif_decode_alpha_gto(memory,pc);
      }
    else
      {
        raw41lif_decode_digits(memory,pc);
      }
  }


int raw41lif_decode_row_c(unsigned char *memory, int *pc)
  {
    if(memory[*pc]>=0xce)
      {
        /* It's one of the odd 2 instructions */
        raw41lif_swap_lbl(pc);
        return(0); /* This can't be the end */
      }
    else
      {
        /* It's a global label or an end */
        return(raw41lif_global_end(memory,pc));
      }
  }


void raw41lif_decode_text(unsigned char *memory, int *pc)
  {
    (*pc)+=(memory[*pc]&0xf)+1; /* And pc to after the string */
  }


int raw41lif_decode_instruction(unsigned char *memory, int *pc)
  {
    /* Instructions with the same high nybble tend to have similar
       formats, so decode based on this */
    int end_flag; /* Set to signal end of program */
    end_flag=0;
    switch(memory[*pc]>>4)
      {
        case 0: raw41lif_short_lbl(pc);
                break;
        case 1: raw41lif_decode_row_1(memory,pc);
                break;
        case 2:
        case 3: raw41lif_short_sto_rcl(pc);
                break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8: raw41lif_decode_1_byte(pc);
                break;
        case 9:
        case 0xa: raw41lif_decode_2_byte(pc);
                  break;
        case 0xb: raw41lif_short_gto(pc);
                  break;
        case 0xc: end_flag=raw41lif_decode_row_c(memory,pc)?1:end_flag;
                  break;
        case 0xd:
        case 0xe: raw41lif_decode_gto_xeq(pc);
                  break;
        case 0xf: raw41lif_decode_text(memory,pc);
                  break;
      }
    return(end_flag);
  }
             


void raw41lif_decode_prog(unsigned char *memory, int length, int *prog_length)
  {
    int pc; /* current program counter */
    int end_flag=0; /* End of program detected */
    /* Scan throguh the loaded program and determine program length */
    pc=0;
    while ((pc<length) && (!end_flag))
      {
        end_flag=raw41lif_decode_instruction(memory,&pc);
      }
    *prog_length= pc;
  }



int raw41lif(int argc, char**argv)
  {
    unsigned char dir_entry[ENTRY_SIZE]; /* New directory entry */
    char lif_filename[NAME_LEN+1];
    int file_length; /* file length */
    int prog_length; /* program length */
    int checksum;    /* checksum */
    unsigned char memory[MEMORY_SIZE]; /* HP41 program memory */
    FILE *fp;

    int i,j,option;

    /* command line options */
    while ((option=getopt(argc,argv,"?"))!=-1)
      {
        switch(option)
          {
            case '?' : raw41lif_usage();
                       return(RETURN_OK);
          }
      }

    if(argc!=2 && argc!=3)
       {
         raw41lif_usage();
         return(RETURN_ERROR);
       }

    /* Check file name */
    if(check_filename(argv[optind],0)==0)
      {
        fprintf(stderr,"Illegal LIF file name\n");
        return(RETURN_ERROR);
      }

    /* Pad the filename with spaces */
    pad_name(argv[1],lif_filename);
    debug_print("LIF filename: %s\n", lif_filename);
    optind++;

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



    SETMODE_STDOUT_BINARY;

    file_length=read_prog(fp,memory); /* Read in the program file */
    raw41lif_decode_prog(memory,file_length,&prog_length);
    debug_print("prog_length %d\n", prog_length);

    /* create and write directory entry */
    create_entry(dir_entry,lif_filename,0xE080,0,prog_length+1,0);
    /* Implementation bytes for HP41 files */
    dir_entry[28]=prog_length >> 8;
    dir_entry[29]=prog_length & 0xff;
    dir_entry[30]=0x00;
    dir_entry[31]=0x20;
    for(i=0;i< ENTRY_SIZE; i++) {
       putchar((int) dir_entry[i]);
    }
    debug_print("%s\n","LIF directory entry written");
    /* copy file */
    checksum=0;
    for(i=0; i< prog_length; i++) {
       putchar((int) memory[i]);
       checksum+= (int) memory[i];
    }
    debug_print("program %d bytes written\n",prog_length);
    checksum= checksum & 0xff;
    debug_print("checksum %x\n",checksum);
    putchar((int) checksum);
    j= SECTOR_SIZE- ((prog_length+1) % SECTOR_SIZE);
    for (i=0; i<j;i++) putchar(0);
    debug_print("trailer %d bytes written\n",j);
    if(fp!=stdin)fclose(fp);
    return(RETURN_OK);
  }
