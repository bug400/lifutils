/* raw41lif.c -- a filter to process HP41 raw files  files */
/* 2014 Joachim Siebold and placed under the GPL */

/* raw41lif reads hp41 program raw files and creates a lif file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include "lif_create_entry.h"
#include "lif_dir_utils.h"
#include "lif_const.h"

#define MEMORY_SIZE 4096

#define TRUE 1

#define DEBUG 0
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

void usage(void)
{
   fprintf(stderr,
   "Usage: raw41lif LIF-filename \n");
   fprintf(stderr,"      (Input comes from standard input)\n");
   fprintf(stderr,"      (Output goes to standard output)(\n");
   fprintf(stderr,"\n");
   fprintf(stderr,"      the LIF-filename must not exceed 10 characters");
   fprintf(stderr,"\n");
   exit(1);
}


int read_prog(unsigned char *memory)
/* read program into memory */
  {
    int byte_counter;
    int byte;

    byte_counter=0; /* Start loading at start of memory */
    while(((byte=getchar())!=EOF) && (byte_counter<MEMORY_SIZE))
      {
        memory[byte_counter]=byte;
        byte_counter++;
      }
    return(byte_counter); /* number of bytes read */
  }


void short_lbl(unsigned char *memory, int *pc)
  {
    (*pc)++; /* inc program counter */
  }


void short_sto_rcl(unsigned char *memory, int *pc)
  {
    (*pc)++; /* and pc */
  }


void short_gto(unsigned char *memory, int *pc)
  {
    (*pc)+=2; /* inc program counter */
  }


void swap_lbl(unsigned char *memory, int *pc)
  {
    (*pc)+=2; /* And pc */
  }


int global_end(unsigned char *memory, int *pc)
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


void decode_gto_xeq(unsigned char *memory, int *pc)
  {
    (*pc)+=3; /* This is a 3 byte instruction */
  }

void decode_alpha_gto(unsigned char *memory, int *pc)
  {
    /* 'Extend Your HP41' section 15.7 says that if the high nybble of 
       the second byte is not f, then the pc is only incremented by 2 */
    (*pc)+=((memory[(*pc)+1]>>4)==0xf)?(memory[(*pc)+1]&0xf)+2:2;
  }


void decode_digits(unsigned char *memory, int *pc)
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



void decode_1_byte(unsigned char *memory, int *pc)
  {
    (*pc)++; /* Move the program counter */
  }



void decode_2_byte(unsigned char *memory, int *pc)
  {
   (*pc)+=2; /* Move pc on 2 bytes */
  }


void decode_row_1(unsigned char *memory, int *pc)
  {
    if(memory[*pc]>=0x1d)
      {
        /* It's an alpha GTO/XEQ */
        decode_alpha_gto(memory,pc);
      }
    else
      {
        decode_digits(memory,pc);
      }
  }


int decode_row_c(unsigned char *memory, int *pc)
  {
    if(memory[*pc]>=0xce)
      {
        /* It's one of the odd 2 instructions */
        swap_lbl(memory,pc);
        return(0); /* This can't be the end */
      }
    else
      {
        /* It's a global label or an end */
        return(global_end(memory,pc));
      }
  }


void decode_text(unsigned char *memory, int *pc)
  {
    (*pc)+=(memory[*pc]&0xf)+1; /* And pc to after the string */
  }


int decode_instruction(unsigned char *memory, int *pc)
  {
    /* Instructions with the same high nybble tend to have similar
       formats, so decode based on this */
    int end_flag; /* Set to signal end of program */
    end_flag=0;
    switch(memory[*pc]>>4)
      {
        case 0: short_lbl(memory,pc);
                break;
        case 1: decode_row_1(memory,pc);
                break;
        case 2:
        case 3: short_sto_rcl(memory,pc);
                break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8: decode_1_byte(memory,pc);
                break;
        case 9:
        case 0xa: decode_2_byte(memory,pc);
                  break;
        case 0xb: short_gto(memory,pc);
                  break;
        case 0xc: end_flag=decode_row_c(memory,pc)?1:end_flag;
                  break;
        case 0xd:
        case 0xe: decode_gto_xeq(memory,pc);
                  break;
        case 0xf: decode_text(memory,pc);
                  break;
      }
    return(end_flag);
  }
             


void decode_prog(unsigned char *memory, int length, int *prog_length)
  {
    int pc; /* current program counter */
    int end_flag=0; /* Emd of program detected */
    /* Scan throguh the loaded program and determine program length */
    pc=0;
    while ((pc<length) && (!end_flag))
      {
        end_flag=decode_instruction(memory,&pc);
      }
    *prog_length= pc;
  }



int main(int argc, char**argv)
  {
    unsigned char dir_entry[ENTRY_SIZE]; /* New directory entry */
    char lif_filename[NAME_LEN+1];
    int file_length; /* file length */
    int prog_length; /* program length */
    int checksum;    /* checksum */
    unsigned char memory[MEMORY_SIZE]; /* HP41 program memory */

    int i,j;
#ifdef _WIN32
    setmode(fileno(stdin), O_BINARY);
    setmode(fileno(stdout), O_BINARY);
#endif


    /* Process LIF-filename from command line */
    if(argc != 2) {
       usage();
       exit(1);
    }

    /* Check file name */
    if(check_filename(argv[1])==0) 
      {
        fprintf(stderr,"Illegal file name\n");
        exit(1);
      }
    /* Pad the filename with spaces */
    pad_name(argv[1],lif_filename);

    debug_print("LIF filename: %s\n", lif_filename);
    file_length=read_prog(memory); /* Read in the program file */
    decode_prog(memory,file_length,&prog_length);
    debug_print("prog_length %d\n", prog_length);

    /* create and write directory entry */
    create_entry(dir_entry,lif_filename,0xE080,0,prog_length+1,0);
    /* Implementation bytes for HP41 files */
    dir_entry[26]=0x80;
    dir_entry[27]=0x01;
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
    exit(0);
  }
