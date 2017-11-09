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
#include "lif_create_entry.h"
#include "lif_dir_utils.h"
#include "lif_const.h"

#define TRUE 1

#define DEBUG 0
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

void usage(void)
{
   fprintf(stderr, "Usage:textlif LIF-filename \n");
   fprintf(stderr,"      (Input comes from standard input)\n");
   fprintf(stderr,"      (Output goes to standard output)(\n");
   fprintf(stderr,"\n");
   fprintf(stderr,"      the LIF-filename must not exceed 10 characters");
   exit(1);
}

void output_line(unsigned char * line, int l)
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

int main(int argc, char**argv)
{
    int length;  /* length of file in bytes without length */
    int lines;   /* numver of lines in file */
    int file_length; /* length of LIF text file */
    int leftover_bytes; /* to fill entire block */
    unsigned char dir_entry[ENTRY_SIZE]; /* New directory entry */
    unsigned char line[LINE_LENGTH];     /* line buffer */
    unsigned char filebuffer[MAX_LENGTH]; /* file buffer */
    unsigned char ch,lastchar;
    char lif_filename[NAME_LEN+1];
    int c;
    int i,j, buflen;

    SETMODE_STDOUT_BINARY;

    lines= length= 0;

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

    /* copy from stdin to file buffer, count length and lines */
    lastchar= (char) 0x0;
    i=0;j=0;
    while (i < MAX_LENGTH) {
      c=getchar();
      if (c == EOF) break;
      j++;
      filebuffer[i]= (unsigned char) c;
      i++;
      if (c== (int) '\n') {
         lines++;
         j--;
         if(j & 1) {
            j++;
         }
         length+=j; 
         j=0;
      }
      lastchar= (unsigned char) c;
    }
    buflen=i;

    /* last line without nl */
    if(lastchar != (int) '\n') length= length+j;
    debug_print("LIF file lines %d\n",lines);

    /* file length is length of all records + size field of all records
       + end of file mark */
    file_length= (length+lines*2)+2;
    debug_print("LIF file length %d\n",file_length);

    /* check total length */
    if (file_length > MAX_LENGTH) {
       fprintf(stderr,"File too large\n");
       exit(1);
    }

    /* create and write directory entry */
    create_entry(dir_entry,lif_filename,1,0,file_length,0);
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
          output_line(line,j);
          j=0;
       } else {
          line[j]=ch;
          j++;
       }
    } 
    /* write eof mark */
    if (lastchar != '\n')
    {
       output_line(line,j);
    }
    output_line((unsigned char *) NULL, 0);
    /* fill up to block lenght */
    if((leftover_bytes=file_length%SECTOR_SIZE))
    {
        for(j=leftover_bytes;j<SECTOR_SIZE;j++) putchar((int) 0);
    }
    exit(0);
}
