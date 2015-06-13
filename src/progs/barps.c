/* barps.c -- print an HP41 barcode file on a Postscript printer */
/* 2001 A. R. Duell, and placed under the GPL */

#include<stdio.h>
#include <stdlib.h>
#include <fcntl.h>

/* Parameters for postscript page layout */
#include"ps_const.h"

/* maximum length of barcode row in bytes */
#define BARCODE_LENGTH 16

void function_def(void)
/* define postscript functions needed for printing barcode */
  {
    /* Document header */
    printf("%%!PS-Adobe-3.0\n");
    printf("%%Output from barps\n\n");
    /* title font */
    printf("/tf { /Times-Bold findfont 16 scalefont setfont } def\n");
    /* page number font */
    printf("/pf { /Times-Roman findfont 10 scalefont setfont } def\n");
    /* row label font */
    printf("/rf { /Times-Roman findfont 8 scalefont setfont } def\n");
    /* These bar-drawing routines expect the coordinates of the top left 
       corner on the experession stack. They leave the coordinates of the 
       top left corner of the next bar on the expression stack. Therefore,
       they are used by putting the coordinates of the start of the barcode
       row on the stack, then executing nb and wb as appropriate, then  
       pop pop at the end of the row */
    /* draw a narrow (0) bar x y -- nx y */
    printf("/nb {\n");
    printf("2 copy moveto %g 0 rlineto 0 %g rlineto\n",NARROW_W,BARCODE_HEIGHT);
    printf("%g 0 rlineto 0 %g rlineto closepath\n",-NARROW_W,-BARCODE_HEIGHT);
    printf("fill exch %g add exch } def\n",NARROW_SP);
    /* draw a wide (1) bar x y -- nx y */
    printf("/wb {\n");
    printf("2 copy moveto %g 0 rlineto 0 %g rlineto\n",WIDE_W,BARCODE_HEIGHT);
    printf("%g 0 rlineto 0 %g rlineto closepath\n",-WIDE_W,-BARCODE_HEIGHT);
    printf("fill exch %g add exch } def\n",WIDE_SP);
  }

void print_title(char *title, int page_no)
/* print title at top of page */
  {
    printf("\n%% page %d\n",page_no);
    printf("%d %d moveto tf ( %s ) show\n",LEFT_MARGIN,TITLE_Y,title);
    printf("pf ( page %d ) show rf\n",page_no);
  }

void barcode_byte(unsigned char output_byte)
/* Print one byte as barcode */
  {
    int bit; /* bit number in byte */
    /* MSB comes first */
    for(bit=7; bit>=0; bit--)
      {
        /* convert byte to binary, one bit at a time, and print the 
           appropriate bar */
        printf("%s ",output_byte&(1<<bit)?"wb":"nb");
       }
     printf(" %% %02x\n",output_byte);
  }

void barcode_row(unsigned char *data, int length, int row, int row_on_page)
/* Print row title and one row of barcode */
  {
    int byte_counter; /* counter for barcode bytes */
    int label_y; /* y-position of this title */
    int barcode_y; /* y-position of this barcode */

    /* calculate where to put it */
    label_y = LABEL_Y-(ROW_SPACING*row_on_page);
    barcode_y=BARCODE_Y-(ROW_SPACING*row_on_page);

    /* print row title */
    printf("%% row %d\n",row);
    printf("%d %d moveto ( Row %d ) show\n",LEFT_MARGIN,label_y,row);

    /* print barcode */
    printf("%d %d nb nb\n",LEFT_MARGIN,barcode_y); /* header */
    for(byte_counter=0; byte_counter<length; byte_counter++)
      {
        barcode_byte(data[byte_counter]); /* barcode data */
      }
    printf("wb nb pop pop\n"); /* trailer */
  }

void print_barcode(char *title)
/* Print a barcode file */
  {
    int row; /* current barcode row */
    int row_on_page; /* row position on current page _starting at 0_ */
    int page; /* current page */
    int length; /* length of current row in bytes */
    int printed_something; /* something on the current page */
    unsigned char data[BARCODE_LENGTH];

    row=1; /* Start at row 1 */
    printed_something=0; /* nothing printed yet */

    while((length=getchar())!=EOF)
      {
        /* Set up parameters for this row */
        length&=0xf; /* true barcode length is low nybble+1 */
        length++;
        page=((row-1)/ROWS)+1;
        row_on_page=(row-1)%ROWS;
        
        /* Read in data */
        if(fread(data,sizeof(unsigned char),length,stdin)!=length)
          {
            fprintf(stderr,"Unexpected EOF reading barcode data\n");
            exit(1);
          }
        printed_something=1;
        if(!row_on_page)
          {
            /* if this is the first row on the page, print the page title */
            print_title(title,page);
          }
        barcode_row(data,length,row,row_on_page);
        row++;
        if(row_on_page==ROWS-1)
          {
            /* This is the last row on this page */
            printf("\nshowpage\n");
            printed_something=0;
          }
      }
    if(printed_something)
      {
        /* If something has been printed without a 'showpage', add one */
        printf("\nshowpage\n");
      }
  }

void usage(void)
  {
    fprintf(stderr,"usage : barps [title]\n");
    exit(1);
  }

int main(int argc, char **argv)
  {
    char *title; /* title for barcode pages */

#ifdef _WIN32
    setmode(fileno(stdin), O_BINARY);
#endif

    if(argc>2)
      {
        usage();
      }

    title=(argc==2)?argv[1]:"HP41 Barcode";

    function_def();
    print_barcode(title);
    exit(0);
  }

