/* rom41cat.c -- print a catalogue of the functions in an unscrambled 
                 HP41 ROM dump */
/* 2001 A. R. Duell, and placed under the GPL */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include "xrom.h"
#include "config.h"

/* Each 4K block of an HP41 ROM module starts with the FAT (Function Address
   Table). This contains the entry points for the functions in the ROM, 
   and whether they are in Focal (user code) or Mcode (machine code). 
   For machine code functions, the words preceding the entry point
   are the function name, using the HP41 'display' character set. For
   user code functions, the entry point is at a global label instruction, 
   the text of which is the name of the function.

   Further details of the format of the FAT (and of HP41 ROMs in general) 
   can be found in 'HP-41 Mcode for Beginners' by Ken Emery */

/* Buffer to store ROM image */
unsigned int rom[16384];
  
unsigned int read_rom(void)
/* Read a ROM image from standard input */
  {
    unsigned int address; /* current ROM image address */ 
    unsigned char word[2]; /* 2 bytes for each 10 bit word */

    address=0;
    /* read in the ROM words */
    while(fread(word, sizeof(unsigned char), 2, stdin)==2)
      {
         rom[address++]=(word[0]<<8)+word[1]; /* store the word */
      }
    return(address); /* And return the number of words read */
  }

unsigned char disp2asc (unsigned char disp_char)
/* Attempt to translate HP41 'display' characters into ascii */
  {
    /* Minor 'bug' : The geese are displayed as the corresponding punctuation
       characters */
    unsigned char row4[16] = 
    {127,'a','b','c','d','e',0,96,6,4,5,1,12,29,126,13};
    if (disp_char>79) { return('?');} /* illegal char */
    if (disp_char<32) { return(disp_char+64); } /* upper case ascii */
    if (disp_char<64) { return(disp_char); } /* digits and punctuation */
    return(row4[disp_char-64]); /* misc characters */
  }


void display_names(unsigned int rom_size, int xrom_flag, int utf_flag)
/* Go through the FAT and print out the names */
  {
    unsigned int xrom; /* XROM id for this page */
    unsigned int n_funcs; /* number of functions in this 4K page */
    unsigned int func; /* current function number */
    unsigned int n_pages; /* number of 4K pages in this image */
    unsigned int page; /* current page */ 
    unsigned int fat_address; /* address of FAT entry for this function */
    unsigned int start_address; /* start address of this function */
    unsigned int addr;
    unsigned int focal_flag; /* Is this function in focal? */
    unsigned char c;
    int l,i;

    unsigned char fname[MAX_ALPHA];
    
    n_pages = (rom_size+4095)/4096; /* number of 4K pages to look at */
    /* look at each page in turn */
    if(! xrom_flag) printf("Number of pages %d ROM size %d\n",n_pages, rom_size);
    for(page=0; page<n_pages; page++)
      {
        xrom = rom[page*4096]; /* XROM id number */
        n_funcs = rom[page*4096+1]; /* number of functions in the FAT */

        if(!xrom_flag) printf("Page %d, Number of Functions %d\n",page+1,n_funcs);
        for(func=0; func<n_funcs; func++)
          {
            fat_address=page*4096 + 2*func + 2;
            start_address=((rom[fat_address]&0x7f)<<8) 
                         +(rom[fat_address+1]&0xff)
                         +page*4096;
            // if the start address is outside the rom address space print message on stderr
            if(start_address> rom_size) {
                fprintf(stderr,"%d %d function addr %x outside of ROM address space\n",xrom,func,start_address);
                continue;
            }
            focal_flag=rom[fat_address]&0x200;
            if(focal_flag) {
               l=rom[start_address+2]-0xF1; /* get the string length nybble */
               for(i=0;i<l;i++) {
                  c= rom[start_address+i+4]&0x7f;
                  fname[i]=((c==' ')?'_':c);
               }
            } else {
               l=0;
               addr= start_address;
               do {
                  c=disp2asc((rom[--addr]&0x7f));
                  fname[l]=((c==' ')?'_':c);
                  l++;
               }
               while(!(rom[addr]&0x80));
            }
            if(xrom_flag) {
               printf("%d %d ",xrom,func);
               if(focal_flag) {
                   printf("XROM'%s'",to_hp41_string(fname,l,0));
                   if(utf_flag && has_special_characters(fname,l)) {
                      printf(" XROM'%s'",to_hp41_string(fname,l,1));
                   }
               } else {
                   printf("%s",to_hp41_string(fname,l,0));
                   if(utf_flag && has_special_characters(fname,l)) {
                      printf(" %s",to_hp41_string(fname,l,1));
                   }
               }
               printf("\n");
           } else {
              printf("XROM %02d,%02d ",xrom,func);
              /* for user output, print the ID, language and entry point */
              printf("Entry = %04x ", start_address);
              printf("(%s) ",focal_flag?"Focal":"Mcode");
              printf("%s\n",to_hp41_string(fname,l,utf_flag));
           }
        }
      }
  }

void usage(void)
  {
    fprintf(stderr,"usage : rom41cat [-x]\n");
    fprintf(stderr,"        -x : output in XROM file format\n");
    exit(1);
  }

int main(int argc, char **argv)
  {
    unsigned int rom_size; /* size of input image */
    int xrom_flag=0; /* output in XROM file format? */
    int utf_flag=0;  /* output names with utf characters */
    int option; /* current option character */

    SETMODE_STDIN_BINARY;

    /* Process command line options */
    optind=1;
    while((option=getopt(argc,argv,"ax?"))!=-1)
      {
        switch(option)
          {
            case 'x' : xrom_flag=1;
                       break;
            case 'a' : utf_flag=1;
                       break;
            case '?' : usage();
          }
      }
    if(optind!=argc)
      {
        /* There shouldn't be any other arguments */
        usage();
      }

    /* Read in the ROM */
    rom_size=read_rom();
    /* And print the function names */
    display_names(rom_size,xrom_flag, utf_flag);
    exit(0);
  }

