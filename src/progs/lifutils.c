/* lifutils.c main program
   2024 J. Siebold and placed under the GPL */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <config.h>
#include "hp41_tables.h"
#include "version.h"
#include "lifutils.h"
#include "lif_dir_utils.h"

#define PROGNAME_LEN 15
#define HELP_LEN 100

typedef int (*func_type)(int argc, char **argv);

typedef struct  {
   func_type f;
   char name[PROGNAME_LEN];
   char help[HELP_LEN];
} func ;

func functions[]= {
   { .f=&barprt, .name= "barprt",.help="diagnostic dump of a HP-41 intermediate barcode file" },
   { .f=&barps, .name= "barps",.help="convert an intermediate barcode file to PostScript" },
   { .f=&comp41, .name= "comp41",.help="compile a HP-41 FOCAL program" },
   { .f=&decomp41, .name= "decomp41",.help="decompile a HP-41 program raw file" },
   { .f=&er41rom, .name= "er41rom",.help="unscramble an Eramco ROM image" },
   { .f=&hx41rom, .name= "hx41rom",.help="unscramble SDATA file containing a scrambled HEPAX rom image" },
   { .f=&inp41, .name= "inp41",.help="decode a HP41 hexadecimal program" },
   { .f=&in71, .name= "in71",.help="read a program from a HP-71B" },
   { .f=&key41, .name= "key41",.help="display an HP-41 key assignment file" },
   { .f=&lexcat71, .name= "lexcat71",.help="display main and text table information of a HP-71 lex file" },
   { .f=&lexcat75, .name= "lexcat75",.help="display information about a HP-75 lex file" },
   { .f=&lifdir, .name= "lifdir",.help="display the directory of a LIF image file" },
   { .f=&liffix, .name= "liffix",.help="fix the medium size information of a LIF image file" },
   { .f=&lifget, .name= "lifget",.help="extract a file from a LIF image file" },
   { .f=&lifinit, .name= "lifinit",.help="initialize a LIF image file" },
   { .f=&liflabel, .name= "liflabel",.help="label a LIF image file" },
   { .f=&lifmod, .name= "lifmod",.help="read contents and info of HP-41 module files" },
   { .f=&lifheader, .name= "lifheader",.help="show the header information of a LIF file" },
   { .f=&lifpack, .name= "lifpack",.help="pack a LIF image file" },
   { .f=&lifpurge, .name= "lifpurge",.help="purge a file from a LIF image file" },
   { .f=&lifput, .name= "lifput",.help="put a LIF file into a LIF image file" },
   { .f=&lifraw, .name= "lifraw",.help="remove the LIF file header from a LIF file" },
   { .f=&lifrename, .name= "lifrename",.help="rename a file in  a LIF image file" },
   { .f=&lifstat, .name= "lifstat",.help="display properties of a LIF image file" },
   { .f=&liftext, .name= "liftext",.help="translate a HP LIF text file into an ASCII text file" },
   { .f=&liftext75, .name= "liftext75",.help="translate a raw HP-75 text file into an ASCII text file" },
   { .f=&outp41, .name= "outp41",.help="produce a HP41 hexadecimal program" },
   { .f=&out71, .name= "out71",.help="send a program to a HP-71B" },
   { .f=&prog41bar, .name= "prog41bar",.help="produce an intermediate barcode file from a HP-41 program raw file"},
   { .f=&raw41lif, .name= "raw41lif",.help="convert a HP41C program raw file to a LIF file"},
   { .f=&regs41, .name= "regs41",.help="display a HP41 raw file as registers" },
   { .f=&rom41cat, .name= "rom41cat",.help="display the function names in a HP41 ROM image" },
   { .f=&rom41er, .name= "rom41er",.help="convert a unscrambled HP-41 rom file to a LIF file for the Eramco MLDL-OS" },
   { .f=&rom41hx, .name= "rom41hx",.help="convert a unscrambled HP-41 rom file to a LIF file of type sdata that can be uploaded to HEPAX RAM" },
   { .f=&rom41lif, .name= "rom41lif",.help="convert a unscrambled HP-41 rom file to a LIF file of type sdata" },
   { .f=&sdata, .name= "sdata",.help="interpret a raw SDATA file" },
   { .f=&sdatabar, .name= "sdatabar",.help="convert a raw SDATA file to an intermediate barcode file" },
   { .f=&stat41, .name= "stat41",.help="display information of a raw HP41 status file" },
   { .f=&textlif, .name= "textlif",.help="convert an ASCII file to a LIF text file" },
   { .f=&textlif75, .name= "textlif75",.help="convert an ASCII file to a HP75 text file" },
   { .f=&wall41, .name= "wall41",.help="extract information from a raw HP41 Write All file" },
   { .f=&wcat41, .name= "wcat41",.help="display a catalogue of the contents of a raw HP-41 write-all file" },
};


/*
   common subroutines

*/
void add_to_checksum(int *checksum, unsigned char data, unsigned char use_carry)
/* Add data to checksum using the end-around-carry algorithm */
  {
    (*checksum) += data; /* add the new data */
    if((*checksum)>255 && use_carry)
      {
        /* There is a carry */
        (*checksum)++; /* add it end-around */
      }
    (*checksum) %= 256; /* keep only the low byte */
  }

int read_prog(FILE *fp,unsigned char *memory)
/* Read in HP41 program from standard input and return length */
  {
    int byte_counter; /* current address */
    int byte; /* byte read */

    byte_counter=0; /* start loading at start of memory */
    while (((byte=getc(fp))!=EOF) && (byte_counter<MEMORY_SIZE))
      {
        memory[byte_counter++]=byte;
      }
    return(byte_counter); /* number of bytes read */
  }

void print_char_hex(unsigned char c)
/* print a character, 'normally if printable, otherwise in hex */
  {
    if(isprint(c))
      {
        putchar(c);
      }
    else
      {
        printf("0x%2.2X",c);
      }
  }


void print_byte_hex(unsigned char byte)
/* Print byte in hex to standard output */
  {
    printf("%02X",byte);
  }

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

int correct_for_gap(int value)
/* Correct for the 'missing registers' not saved in a Write-All file */
  {
    if(value>0xf)
      {
        value-=176;
      }
    return(value);
  }

void lifutils_usage(void)
{
   int i;

   fprintf(stderr,"\nLIFUTILS %s\n\n",VERSION_STR);
   fprintf(stderr,"Usage: lifutils [-v] [-?]\n");
   fprintf(stderr,"       -v Output version string\n");
   fprintf(stderr,"or\n");
   fprintf(stderr,"       lifutils <program> <parameters ...>\n\n");
   fprintf(stderr,"Available programs are:\n\n");
   for(i=0;i<(int)(sizeof(functions)/sizeof(func));i++) {
      fprintf(stderr,"%15.15s %s\n",functions[i].name,functions[i].help);
   }
   fprintf(stderr,"\nUse: lifutils <program> -? for help for a specific program\n");
}

int main(int argc, char **argv)
{
   int i,ret;
   char progname[PROGNAME_LEN];

   /* no parameter */
   if(argc==1) {
      lifutils_usage();
      return(EXIT_ERROR);
   }
   /* output version string */
   if(strcmp(argv[1],"-v")==0) {
      printf("%02d%02d%02d\n",VERSION0,VERSION1,VERSION2);
      exit(EXIT_OK);
   }
   /* output help */
   if(strcmp(argv[1],"-?")==0) {
      lifutils_usage();
      exit(EXIT_OK);
   }
   if(strlen(argv[1])>PROGNAME_LEN) {
      lifutils_usage();
      exit(EXIT_ERROR);
   }
   strcpy(progname,argv[1]);
   for(i=0;i<(int)strlen(progname);i++) {
      progname[i]=tolower(progname[i]);
   }

   for(i=0;i<(int) (sizeof(functions)/sizeof(func));i++) {
      if(strcmp(progname,functions[i].name)==0) {
         ret=functions[i].f(argc-1,&argv[1]);
         if(ret== RETURN_ERROR) {
            exit(EXIT_ERROR);
         } else {
            exit(EXIT_OK);
         }
      }
   }
   fprintf(stderr,"illegal program name %s\n",argv[1]);
   lifutils_usage();
   exit(EXIT_ERROR);
}
