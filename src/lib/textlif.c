/* text.c -- a filter to process HP text (LIF1) files */
/* 2000 A. R. Duell, , 2014 J. Siebold and placed under the GPL */

/* An HP text file, also known as a 'LIF file type 1' consists of a 
   number of records. Each record starts with a 2 byte length (high byte 
   first), and contains that number of bytes. A record length of 0xFFFF 
   indicates the end of the file. A record length of 0xFFFE indicates a 
   null record.

   Note that if the record length is odd, then an extra dummy byte is placed
   at the end of the record. Thus the total length (header+data) of any 
   record is even.

   See HP71 Software IDS volume 1, section 11.2.8.1 for further details

   This filter reads a textfile and creates a LIF text files with a 
   directory header */

#include <stdio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "config.h"
#include "lifutils.h"
#include "lif_create_entry.h"
#include "lif_dir_utils.h"
#include "lif_const.h"


#define DEBUG 0

void textlif_usage(void)
{
   fprintf(stderr,"Usage : lifutils textlif [-s size] [-l] LIFFILENAME INPUTFILE > output file\n");
   fprintf(stderr,"        or\n");
   fprintf(stderr,"        lifutils textlif [-s size] [-l] LIFFILENAME < input file > output file\n");
   fprintf(stderr,"        Convert an ASCII file to a LIF text file\n");
   fprintf(stderr,"        -s Specifies file size in HP-41 registers.\n");
   fprintf(stderr,"           If value 0 is specified, the number of registers is\n");
   fprintf(stderr,"           determined automatically from the input text file\n");
   fprintf(stderr,"           size. Note: this parameter is only needed if the resulting\n");
   fprintf(stderr,"           file shall be used on the HP-41!\n");
   fprintf(stderr,"        -l Relax file name checking, allow underscores in file names.\n\n");
   fprintf(stderr,"\n");
}

void textlif_output_line(unsigned char * line, int l)
{
   /* copy l bytes from line buffer to standard output */

   int i;

   /* EOF */
   if ( line == (unsigned char *) NULL) {
      debug_print("%s\n","EOF Mark");
      putchar(0xff);
      putchar(0xff);
      return;
   }
   /* output record with 2 byte record length header */
   debug_print("length= %d ",l);
   putchar((int) (l >> 8));
   putchar((int) (l & 0xff));
   debug_print("%2.2x",(int) l >> 8);
   debug_print("%2.2x ",(int) l & 0xff);
   for(i=0;i<l;i++) {
      putchar((int) line[i]);
      debug_print("%c", line[i]);
   }
   if(l & 1) {
      putchar((int) 0);
      debug_print("%s","(Dummy Byte)");
   }
   debug_print("%s\n","(EOL)");
}

int textlif(int argc, char**argv)
{
    int length;  /* length of file in bytes without length fields including 
                    pad bytes */
    int lines;   /* numver of lines in file */
    int num_characters; /* number of characters in file */
    int file_length; /* length of LIF text file */
    int leftover_bytes; /* to fill entire block */
    int num_reg= -1;    /* file length in registers */
    int min_reg;      /* minimun number of registers needed on the HP-41 */
    char *snum_reg= (char *) NULL;
    int lax; /* option to relax file name checking */
    unsigned char dir_entry[ENTRY_SIZE]; /* New directory entry */
    unsigned char line[LINE_LENGTH];     /* line buffer */
    unsigned char filebuffer[MAX_LENGTH]; /* file buffer */
    unsigned char ch,lastchar;
    char lif_filename[NAME_LEN+1];
    int option;
    int c;
    int i,j,k, buflen;
    FILE *fp;


    lines= length= num_characters=0;
    lax=0;

    /* command line options */
    while ((option=getopt(argc,argv,"s:l?"))!=-1)
      {
        switch(option)
          {
            case 's':  snum_reg=optarg;
                       break;
            case 'l':  lax=1;
                       break;
            case '?' : textlif_usage();
                       return(RETURN_OK);
          }
      }

    if(argc!=2 && argc!=3)
       {
         textlif_usage();
         return(RETURN_ERROR);
       }

    /* Check file name */
    if(check_filename(argv[optind],lax)==0)
      {
        fprintf(stderr,"Illegal LIF file name\n");
        return(RETURN_ERROR);
      }

    /* Pad the filename with spaces */
    pad_name(argv[optind],lif_filename);
    optind++;

    debug_print("LIF filename: %s\n", lif_filename);

    /* get number of registers option */
    if ( snum_reg != (char *) NULL) {
       if (sscanf(snum_reg,"%d",&num_reg) !=1) {
          textlif_usage();
          return(RETURN_ERROR);
       }
       debug_print("file size of HP-41 registers specified %d\n",num_reg);
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
    /* copy from stdin to file buffer, count length and lines */
    lastchar= (char) 0x0;
    i=0;j=0;k=0;
    while (i < MAX_LENGTH) {
      c=getc(fp);
      if (c == EOF) break;
      j++;
      k++;
      filebuffer[i]= (unsigned char) c;
      i++;
      if (c== (int) '\n') {
         lines++;
         j--;
         k--;
         if(j & 1) {  /* odd length, add pad byte */
            j++;
         }
         length+=j; 
         j=0;
         num_characters+=k;
         k=0;
      }
      lastchar= (unsigned char) c;
    }
    buflen=i;

    /* last line without nl */
    if(lastchar != (int) '\n') {
       length+=j;
    } else {
       num_characters+=k;
    }
    debug_print("number of lines in input file %d\n",lines);
    debug_print("number of characters in input file %d\n",num_characters);
    debug_print("number of characters including pad bytes %d\n",length);

    /* file length is length of all records + size field of all records
       + end of file mark */
    file_length= (length+lines*2)+2;
    /* register length is number of characters+number of records+1 
       see HEPAX doc */
    min_reg=((num_characters+lines+1)+6)/7;
    debug_print("LIF file length %d\n",file_length);
    debug_print("minimum length in HP 41 registers  %d\n",min_reg);

    if (num_reg != -1) {
       if (num_reg == 0) {
          num_reg= min_reg;
       } else {
          if (num_reg < min_reg) {
             fprintf(stderr,"Number of HP-41 registers too small, the minimum is %d\n", min_reg);
             if(fp!=stdin) fclose(fp);
             return(RETURN_ERROR);
          }
       }
       if (num_reg > 577) {  /* max file size, see HEPAX documentation */
          fprintf(stderr,"Number of HP-41 registers too large, maximum is 577 registers\n");
          if(fp!=stdin) fclose(fp);
          return(RETURN_ERROR);
       }
    }
    /* check total length */
    if (file_length > MAX_LENGTH) {
       fprintf(stderr,"File too large\n");
       if(fp!=stdin) fclose(fp);
       return(RETURN_ERROR);
    }

    /* create and write directory entry */
    create_entry(dir_entry,lif_filename,1,0,file_length,0);

    /* Store implementation bytes for HP41 text files */
    if (num_reg != -1) {
       dir_entry[28]=num_reg >> 8;
       dir_entry[29]=num_reg & 0xff;
       dir_entry[30]=0x00;
       dir_entry[31]=0x20;
       debug_print("file size of HP-41 registers written %d\n",num_reg);
    }

    for(i=0;i< ENTRY_SIZE; i++) {
       putchar((int) dir_entry[i]);
    }
    debug_print("%s\n","LIF directory entry written");

    /* now copy file */
    lastchar= (char) 0x0;
    j=0;
    for(i=0;i<buflen;i++) {
       ch= filebuffer[i];
       lastchar=ch;
       if (ch == '\n') {
          textlif_output_line(line,j);
          j=0;
       } else {
          line[j]=ch;
          j++;
       }
    } 
    /* write eof mark */
    if (lastchar != '\n')
    {
       textlif_output_line(line,j);
    }
    textlif_output_line((unsigned char *) NULL, 0);
    /* fill up to block lenght */
    if((leftover_bytes=file_length%SECTOR_SIZE))
    {
        for(j=leftover_bytes;j<SECTOR_SIZE;j++) putchar((int) 0);
    }
    if(fp!=stdin)fclose(fp);
    return(RETURN_OK);
}
