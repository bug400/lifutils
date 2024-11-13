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
#include "lifutils.h"
#include "lif_dir_utils.h"
#include"descramble_41.h"



int wcat41_read_wall_file(FILE *fp,unsigned char * memory)
/* Read in the file, return #bytes stored */
  {
    unsigned char rec[SDATA_RECORD_SIZE]; /* file record */
    int byte_pointer=0;
    /* Read in the file */
    while((fread(rec,sizeof(unsigned char),SDATA_RECORD_SIZE,fp)==SDATA_RECORD_SIZE)
           && (byte_pointer<MEMORY_SIZE))
      {
        /* convert each record to a register and store it */
        descramble(rec,memory+byte_pointer);
        byte_pointer+=REGISTER_SIZE;
      }
    return(byte_pointer);
  }

void wcat41_KARs_buffers(unsigned char * memory, int global_end)
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

void wcat41_inc_pc(int *reg, int *byte_offset)
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

unsigned char wcat41_read_byte(unsigned char * memory, int *reg, int *byte_offset)
/* Read next byte from program memory and increment the pointer */
  {
    unsigned char prog_byte;
    prog_byte=memory[(*reg)*7+(*byte_offset)];
    wcat41_inc_pc(reg,byte_offset);
    return(prog_byte);
  }

void wcat41_prog_labels(unsigned char * memory, int curtain, int global_end)
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
        switch((prog_byte=wcat41_read_byte(memory,&reg,&byte_offset))>>4)
          {
            /* Bytes 0x00-0x8f are single-byte instructions, so nothing to
               do. Alpha XEQ/GTO are treated as a single-byte follwed by 
               a string. */
            case 0x9 :
            case 0xa :
            case 0xb : /* 2 byte instructions, skip suffix */
                       wcat41_inc_pc(&reg,&byte_offset);
                       break;
            case 0xc : /* labels, ENDs and a couple of leftovers */
                       wcat41_inc_pc(&reg,&byte_offset);
                       if(prog_byte<=0xcd)
                         {
                           /* It's a label or an end */
                           prog_byte=wcat41_read_byte(memory,&reg,&byte_offset);
                           if(prog_byte&0x80)
                             {
                               /* high bit is set, it's a label */
                               /* skip key assignment */
                               wcat41_inc_pc(&reg,&byte_offset); 
                               putchar(34); /* double quote */
                               for(i=0; i<(prog_byte&0xf)-1;i++)
                                 {
                                   print_char_hex(wcat41_read_byte(memory,&reg,&byte_offset));
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
                       wcat41_inc_pc(&reg,&byte_offset);
                       wcat41_inc_pc(&reg,&byte_offset);
                       break;
            case 0xf : /* strings */
                       for(i=0; i<(prog_byte&0xf); i++)
                         {
                           wcat41_inc_pc(&reg,&byte_offset);
                         }
                       break;
          }
      }
  }

void wcat41_usage(void)
{
   fprintf(stderr,"Usage : lifutils wcat41 [-r] INPUTFILE [> output file]\n");
   fprintf(stderr,"        or\n");
   fprintf(stderr,"      : lifutils wcat41 [-r] < input file [> output file]\n");
   fprintf(stderr,"        Display a catalogue of the contents of a raw HP-41 write-all file\n");
   fprintf(stderr,"        -r skip an existing LIF header of the input file\n");
   fprintf(stderr,"\n");
}



int wcat41(int argc, char **argv)
  {
    /* HP41 memory */
    unsigned char memory[MEMORY_SIZE];

    int bytes_read; /* length of input file */
    int curtain, sreg, global_end; /* Important pointers in HP41 memory */
    int skip_lif=0;
    FILE *fp;

    int option;

    while ((option=getopt(argc,argv,"r?"))!=-1)
      {
        switch(option)
          {
            case 'r' : skip_lif=1;
                       break;
            case '?' : wcat41_usage();
                       return(RETURN_OK);
          }
      }
   if((optind!=argc) && (optind!= argc-1))
      {
        wcat41_usage();
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

    /* skip lif heaeder, note: we have different types of wall files, thus not checked here */
    if(skip_lif) {
       if(skip_lif_header(fp,(char *) NULL)) {
          fclose(fp);
          fprintf(stderr,"Error: not a LIF HP-41 wall file\n");
          return(RETURN_ERROR);
       } 
    } 


    /* Read in the write-all file */
    bytes_read=wcat41_read_wall_file(fp,memory);

    /* Is the length sensible ? */
    if((bytes_read<HP41C_SIZE) || (bytes_read>HP41CV_SIZE))
      {
        /* No it's not */
        fprintf(stderr,"This is not a Write-All file!\n");
        if(fp!=stdin) fclose(fp);
        return(RETURN_ERROR);
      }

    /* Decode 'status register 13' to get the important pointers in memory */
    curtain=correct_for_gap((memory[7*13+4]<<4)+(memory[7*13+5]>>4));
    sreg=correct_for_gap((memory[7*13]<<4)+(memory[7*13+1]>>4));
    global_end=correct_for_gap(((memory[7*13+5]&0xf)<<8)+memory[7*13+6]);

    /* display SIZE and statistical register start */
    printf("SIZE %03d\n",(bytes_read/7)-curtain);
    printf("First statistical register = %03d\n\n",sreg-curtain);

    /* display number of key assignments and buffer IDs */
    wcat41_KARs_buffers(memory,global_end);

    /* display program labels */
    wcat41_prog_labels(memory,curtain,global_end);

    if(fp!=stdin) fclose(fp);
    return(RETURN_OK);
  }
