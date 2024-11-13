/* rom41er.c -- a filter to create Eramco MLDL-OS rom files */
/* 2017 Joachim Siebold and placed under the GPL */

/*  reads a HP-41 rom file and creates a HP-41 data file in Eramco MLDL-OS */
/*  format. See the file er41rom.c for a format description */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include "config.h"
#include "lifutils.h"
#include "scramble_41.h"
#include "lif_create_entry.h"
#include "lif_dir_utils.h"
#include "lif_const.h"

#define DEBUG 0

void rom41er_usage(void)
{
   fprintf(stderr,"Usage : lifutils rom41er LIFFILENAME INPUTFILE > output file \n");
   fprintf(stderr,"        or\n");
   fprintf(stderr,"        lifutils rom41er LIFFILENAME < input file > output file \n");
   fprintf(stderr,"        Convert an unscrambled HP-41 rom file to a LIF file for the Eramco MLDL-OS\n");
   fprintf(stderr,"        -r skip an existing LIF header of the input file\n");
   fprintf(stderr,"\n");
}


int rom41er_read_rom(FILE *fp,unsigned char *memory)
/* read rom into memory, scramble data */
  {
    int byte_counter,j,s,l,t,checksum;
    size_t bytes_read;
    unsigned short rom_data0[ROM_RECORD_SIZE]; /* Input data words */
    unsigned short rom_data1[ROM_RECORD_SIZE]; /* Input data words, read ahead */
    unsigned char hi,lo;

    byte_counter=0; /* Start loading at start of memory */
    s=0;
    /* read in the file data, 1 record at a time */
    bytes_read=fread(rom_data1,sizeof(short),ROM_RECORD_SIZE,fp);
    while(bytes_read== ROM_RECORD_SIZE) {
          memcpy(rom_data0,rom_data1,sizeof(short)* ROM_RECORD_SIZE);
          /* determine checksum, use only the lower 10 bits */
          for(j=0;j< ROM_RECORD_SIZE;j++) {
              l=s;
              t= ((rom_data0[j] & 0x03) <<8) | ((rom_data0[j] & 0xFF00) >> 8);
              s+= t;
              if (s>= 1024) {
                  s= s & 0x3FF;
                  s+=1;
              }
          }
       
          bytes_read=fread(rom_data1,sizeof(short),ROM_RECORD_SIZE,fp);

          /* end of file, then store computed checksum */
          if(bytes_read != ROM_RECORD_SIZE) {
               checksum= (~l & 0x3FF)+1;
               rom_data0[ROM_RECORD_SIZE-1]= ((checksum &0xFF) <<8) | 
                   ((checksum &0xFF00) >> 8);
          }
          /* scramble data */
          if (byte_counter+4 > LIF_ROM41_SIZE) return(RETURN_ERROR);
          l=3;
          memory[byte_counter]=0;
/*
          for(j=0;j< ROM_RECORD_SIZE;j++) {
              fprintf(stderr,"unscrambled %d %x\n",j,rom_data0[j]);
          }
*/
          for(j=0;j< ROM_RECORD_SIZE;j++) {
              lo= (rom_data0[l] & 0xFF00) >> 8;
              hi= rom_data0[l] & 0x0003;
              l-=1;
              memory[byte_counter]= memory[byte_counter] <<2;
              memory[byte_counter]= memory[byte_counter] | hi;
              memory[byte_counter+j+1]= lo;
          }
/*
          for(j=0;j<5;j++) {
             fprintf(stderr,"scrambled %d %x\n",j,memory[byte_counter+j]);
          }
*/
          byte_counter+=5;
    }
    return(byte_counter); /* number of bytes read */
  }

int rom41er(int argc, char**argv)
  {
    unsigned char dir_entry[ENTRY_SIZE]; /* New directory entry */
    char lif_filename[NAME_LEN+1];
    int length; /* length of scrambled code */
    int num_reg; /* number of hpregs in data file */
    unsigned char memory[LIF_ROM41_SIZE]; /* HP41 program memory */
    FILE *fp;

    int i;

    int option;
    while ((option=getopt(argc,argv,"?"))!=-1)
      {
        switch(option)
          {
            case '?' : rom41er_usage();
                       return(RETURN_OK);
          }
      }


    /* LIF-filename is a required parameter */
    if(argc != 2 && argc != 3) {
       rom41er_usage();
       return(RETURN_ERROR);
    }

    /* Check file name */
    if(check_filename(argv[1],0)==0)
      {
        fprintf(stderr,"Illegal LIF file name\n");
        return(RETURN_ERROR);
      }
    /* Pad the filename with spaces */
    pad_name(argv[1],lif_filename);

    lif_filename[NAME_LEN]=(char) 0;
    debug_print("LIF filename: %s\n", lif_filename);

    optind++;
    if((optind!=argc) && (optind!= argc-1))
       {
         rom41er_usage();
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


    SETMODE_STDOUT_BINARY;

    for(i=0;i<LIF_ROM41_SIZE;i++) memory[i]=0;
    length=rom41er_read_rom(fp,memory); /* Read in the program file */
    debug_print("scrambled rom length %d\n", length);
    if(length== RETURN_ERROR) {
       if(fp!=stdin)fclose(fp);
       return(RETURN_ERROR);
    }

    /* create and write directory entry */
    num_reg= length/8;
    create_entry(dir_entry,lif_filename,0xE070,0,length,0);
    /* Implementation bytes for HP41 files */
    dir_entry[28]=num_reg >> 8;
    dir_entry[29]=num_reg & 0xff;
    dir_entry[30]=0x00;
    dir_entry[31]=0x00;
    /* output lif entry */
    for(i=0;i< ENTRY_SIZE; i++) {
       putchar((int) dir_entry[i]);
    }
    debug_print("%s\n","LIF directory entry written");
    for(i=0;i<length;i++) putchar ((int) memory[i]);
    debug_print("ERAMCO Rom image %d bytes written\n",length);
    if(fp!=stdin) fclose(fp);
    return(RETURN_OK);
  }
