/* wcat41.c -- display a catalogue of an HP41 Write-All file */
/* 2000 A. R. Duell, and placed under the GPL */

/* This program reads an HP41 Write-All file (see wall41.c for the 
   format of this file) from standard input and displays the following
   information on standard output :
 
     SIZE and Statistical Register start
     Number of KARs
     ID's of buffers
     Labels and ENDs in user programs  */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"
#include"descramble_41.h"

/* sizes of a file record and HP41 register */
#define RECORD_LENGTH 8
#define REGISTER_LENGTH 7

/* Smallest and largest byte counts of HP41 memory */
#define HP41C_SIZE 560
#define HP41CV_SIZE 2352

/* This is therefore larger than any real HP41 */
#define MEMORY_SIZE 4096

/* HP41 memory */
unsigned char memory[MEMORY_SIZE];

int correct_for_gap(int value)
/* Correct for the missing registers not saved in a Write-All file */
  {
    if(value>0xf)
      {
        value-=176;
      }
    return(value);
  }

int read_file(void)
/* Read in the file, return #bytes stored */
  {
    unsigned char rec[RECORD_LENGTH]; /* file record */
    int byte_pointer=0;
    /* Read in the file */
    while((fread(rec,sizeof(unsigned char),RECORD_LENGTH,stdin)==RECORD_LENGTH)
           && (byte_pointer<MEMORY_SIZE))
      {
        /* convert each record to a register and store it */
        descramble(rec,memory+byte_pointer);
        byte_pointer+=REGISTER_LENGTH;
      }
    return(byte_pointer);
  }

void KARs_buffers(int global_end)
/* Display a count of key assignment registers and the IDs of buffers */
  {
    int byte_counter; /* Address in HP41 Memory */
    int buffno; /* Buffer ID */
    int bufflen; /* Buffer length */
    int kar; /* count of key assignment registers */

    byte_counter=16*7; /* Start in reginster 16 (actually 0xc0) */
    kar=0;

    /* Scan over the KARs and count them */
    while((byte_counter<global_end*7) && (memory[byte_counter]==0xf0))
      {
        kar++; /* This is another KAR */
        byte_counter+=7; /* Point to next register */
      }
    printf("%d KAR%s found\n",kar,(kar==1)?"":"s");

    /* byte_counter now points to the start of the buffer area (if present)
       Scan the buffers and print their IDs */
    while((byte_counter<global_end*7) && memory[byte_counter] &&
          !(memory[byte_counter] % 0x11))
      {
        buffno=memory[byte_counter]&0xf; /* Either nybble is the ID */
        bufflen=memory[byte_counter+1]; /* length in registers */
        printf("Buffer %x (%d register%s)\n",buffno,bufflen,
               (bufflen==1)?"":"s");
        byte_counter +=7*bufflen;
      }
  }

void print_char(unsigned char c)
/* print a character, 'normally if printable, otherwise in octal */
  {
    if(isprint(c))
      {
        putchar(c);
      }
    else
      {
        printf("\\%03o",c);
      }
  }

void inc_pc(int *reg, int *byte_offset)
/* Increment the pointer to a byte of program memory (stored as a register
   and a byte offset in that register). Remember programs are stored 
   backwards, with the first byte in the highest-number register */
  {
    (*byte_offset)++;
    if((*byte_offset)>6)
      {
        /* At the end of a register */
        (*reg)--;
        (*byte_offset)=0;
      }
  }

unsigned char read_byte(int *reg, int *byte_offset)
/* Read next byte from program memory and increment the pointer */
  {
    unsigned char prog_byte;
    prog_byte=memory[(*reg)*7+(*byte_offset)];
    inc_pc(reg,byte_offset);
    return(prog_byte);
  }

void prog_labels(int curtain, int global_end)
/* Display the global labels and ENDs from program memory */
  {
    int reg, byte_offset; /* Program counter */
    unsigned char prog_byte; /* Current program byte */
    unsigned char global_flag=0; /* set when global END found */
    int i; /* loop counter */

    /* Initialise program counter */
    reg=curtain-1; byte_offset=0;

    /* Scan program memory */
    while((!global_flag) && (reg>=global_end))
      {
        /* look at the next byte of the program */
        switch((prog_byte=read_byte(&reg,&byte_offset))>>4)
          {
            /* Bytes 0x00-0x8f are single-byte instructions, so nothing to
               do. Alpha XEQ/GTO are treated as a single-byte follwed by 
               a string. */
            case 0x9 :
            case 0xa :
            case 0xb : /* 2 byte instructions, skip suffix */
                       inc_pc(&reg,&byte_offset);
                       break;
            case 0xc : /* labels, ENDs and a couple of leftovers */
                       inc_pc(&reg,&byte_offset);
                       if(prog_byte<=0xcd)
                         {
                           /* It's a label or an end */
                           prog_byte=read_byte(&reg,&byte_offset);
                           if(prog_byte&0x80)
                             {
                               /* high bit is set, it's a label */
                               /* skip key assignment */
                               inc_pc(&reg,&byte_offset); 
                               putchar(34); /* double quote */
                               for(i=0; i<(prog_byte&0xf)-1;i++)
                                 {
                                   print_char(read_byte(&reg,&byte_offset));
                                 }
                               putchar(34);
                               printf("\n");
                             }
                           else
                             {
                               /* It's an END */
                               if(prog_byte&0x20)
                                 {
                                   /* It's the global END */
                                   printf(".END.\n");
                                   global_flag=1;
                                 }
                               else
                                 {
                                   printf("END\n");
                                 }
                             }
                         }
                       break;
            case 0xd :
            case 0xe : /* 3 byte instructions, skip 2 more bytes */
                       inc_pc(&reg,&byte_offset);
                       inc_pc(&reg,&byte_offset);
                       break;
            case 0xf : /* strings */
                       for(i=0; i<(prog_byte&0xf); i++)
                         {
                           inc_pc(&reg,&byte_offset);
                         }
                       break;
          }
      }
  }

int main(void)
  {
    int bytes_read; /* length of input file */
    int curtain, sreg, global_end; /* Important pointers in HP41 memory */

    SETMODE_STDIN_BINARY;

    /* Read in the write-all file */
    bytes_read=read_file();

    /* Is the length sensible ? */
    if((bytes_read<HP41C_SIZE) || (bytes_read>HP41CV_SIZE))
      {
        /* No it's not */
        fprintf(stderr,"This is not a Write-All file!\n");
        exit(1);
      }

    /* Decode 'status register 13' to get the important pointers in memory */
    curtain=correct_for_gap((memory[7*13+4]<<4)+(memory[7*13+5]>>4));
    sreg=correct_for_gap((memory[7*13]<<4)+(memory[7*13+1]>>4));
    global_end=correct_for_gap(((memory[7*13+5]&0xf)<<8)+memory[7*13+6]);

    /* display SIZE and statistical register start */
    printf("SIZE %03d\n",(bytes_read/7)-curtain);
    printf("First statistical register = %03d\n\n",sreg-curtain);

    /* display number of key assignments and buffer IDs */
    KARs_buffers(global_end);

    /* display program labels */
    prog_labels(curtain,global_end);

    exit(0);
  }
