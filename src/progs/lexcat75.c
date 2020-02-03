/* lexcat75.c -- display the keywords of a HP-75 lex file */
/* 2016 J. Siebold, and placed under the GPL */

/* This program reads an HP-75 lex file (see HP75 Description
   and Entry Points pp 156).

   Note: the lex file header must not be present
*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"

/* This is therefore larger than any real HP75 */
#define LEXFILE_SIZE 65536

/* Don't know the actual size limit, possible overflow is handled below */
#define KEYWORD_LEN  30

/* lex file byte array */
unsigned char lexfile[LEXFILE_SIZE];
/* actual pointer */
unsigned int  byte_ptr;

void usage(void)
{
   fprintf(stderr,
   "Usage: lexcat75 \n");
   fprintf(stderr,"      (Input comes from standard input)\n");
   fprintf(stderr,"      (Output goes to standard output)(\n");
   fprintf(stderr,"\n");
   exit(1);
}

/*
  read up to LEXFILE_SIZE bytes into the array lexfile
*/

void read_file(void)
/* Read in the file, return #nibbles stored */
  {
    int c; /* byte read */
    /* Read in the file */
    while((c=getchar())!=EOF) 
      {
        /* convert each byte to nibbles and store them */
        if(byte_ptr+1 > LEXFILE_SIZE)
          {
             fprintf(stderr,"maximum file size exceeded\n");
             exit(1);
          }
        lexfile[byte_ptr]= c & 0xFF;
        byte_ptr+=1;
      }
    return;
  }

/*
  decode a number from consecutive bytes with variable length
*/

int read_int(start, len)

int start; /* address of first byte */
int len;   /* length in bytes */
  {
    int retval;
    int i;

    retval=0;
    for(i=0;i<len;i++) {
      if(start+i > LEXFILE_SIZE)
        {
           fprintf(stderr,"corrupted file\n");
           exit(1);
        }
      retval= retval | (lexfile[start+i] << (i*8));
    }
    return(retval);
  }
/* 
 analyze and dump a lex table. 
*/

void lex_table()

  {
    int lexfile_id;      /* id of lex file */
    int keyword_ptr;     /* pointer to keyword table */
    int runtime_ptr;     /* pointer to runtime table */  
    int function_ptr;    /* pointer to keyword function code */
    int sec_attr_ptr;    /* pointer to secondary attributes of func/oper */
    int num_params;      /* number of parameters */
    int param_type;      /* parameter type */
    int attr;            /* keyword attributes */
    int class;           /* keyword class */
    int i;               /* keyword counter */
    int shift;           /* keyword attribute mask shift */
    char keyword[KEYWORD_LEN+1];
    int len;             /* keyword length */
    unsigned int c;

    /* check type of file in the file directory */
    byte_ptr=5;
    if(lexfile[byte_ptr]!= 0x4C)  {
        fprintf(stderr,"This is not a HP-75 lex file\n");
        exit(1);
    }
    /* set byte pointer to the beginning of the lex file */
    byte_ptr=18;
    
    /* print the id of the lex file */
    lexfile_id= read_int(byte_ptr,2);    
    fprintf(stdout,"LEX ID: %x (hex)\n",lexfile_id); 

    /* determine offset to the keyword table */
    
    /* set pointer to the beginning of the keyword table */
    keyword_ptr= byte_ptr+read_int(byte_ptr+4,2);

    /* set pointer to the beginning of the runtime tab */
    runtime_ptr= byte_ptr+read_int(byte_ptr+2,2)+2;
    fprintf(stdout,"\nKeywords:\n");

    /* now output keywords and their attribute */
    len=0;
    while (lexfile[keyword_ptr] != 0xFF) {

        /* new keyword, the attribute preceeds the function
            code */
        if(len == 0) {
            function_ptr=read_int(runtime_ptr,2)+byte_ptr;
            runtime_ptr+=2;
            attr= read_int(function_ptr-1,1)& 0xC0;
            class= read_int(function_ptr-1,1)& 0x3F;
        }
        c= lexfile[keyword_ptr];

        /* end of keyword, decode its attribute */
        if (c & 0x80) {
            keyword[len]= (char) c & 0x7F;
            keyword[len+1]='\0';
            
            /* see HP 85 assembler rom documentation 5-19 
                to 5-22 */
            /* BASIC statement, illegal after THEN */
            if(attr == 0xc0) fprintf(stdout,"stmt: ");
            /* BASIC statement, legal after THEN */
            if(attr == 0x80) fprintf(stdout,"STMT: ");
            /* Non programmable */
            if(attr == 0x40) fprintf(stdout,"NOTP: ");
            /* Non BASIC statement */
            if(attr == 0x00) {
               switch (class) {
               case 040: fprintf(stdout,"IMEX: ");
                          break;
               case 041:
               case 042:
               case 044: fprintf(stdout,"OTHR: ");
                          break;
               case 050: fprintf(stdout,"OP#1: ");
                          break;
               case 051: fprintf(stdout,"OP#2: ");
                          break;
               case 052: fprintf(stdout,"OP$1: ");
                          break;
               case 053: fprintf(stdout,"OP$2: ");
                          break;
               case 055: fprintf(stdout,"FNC#: ");
                          break;
               case 056: fprintf(stdout,"FNC$: ");
                          break;
               }
            }
            fprintf(stdout,"%s",keyword);
            /* now decode secondary attributes */
            if(attr == 0x0) {
                sec_attr_ptr= function_ptr-2;
                num_params= (read_int(sec_attr_ptr,1)&0xF0 ) >> 4;
                if(num_params > 0) {
                    fprintf(stdout,"(");
                    shift=2;
                    for(i=0;i<num_params;i++) {
                        param_type=((read_int(sec_attr_ptr,1) & (0x3<<shift))>>shift);
                        switch(param_type) {
                        case 0: fprintf(stdout,"#");
                                break;
                        case 1: fprintf(stdout,"#()");
                                break;
                        case 2: fprintf(stdout,"$");
                                break;
                        }
                        shift-=2;
                        if(shift <0) {
                            shift=6;
                            sec_attr_ptr-=1;
                        }
                        if(i!=num_params-1) fprintf(stdout,",");
                    }
                    fprintf(stdout,")");
                }
            }
            fprintf(stdout,"\n");
            len=0;
        } else {
            if(len== KEYWORD_LEN) {
                fprintf(stderr,"keyword too long\n");
                exit(1);
            } else {
                keyword[len]= (char) c;
                len+=1;
            }
        }
        keyword_ptr+=1;
    }
    fprintf(stdout,"\n");

    /* get the code attributes */
    c=read_int(read_int(byte_ptr+10,2)+byte_ptr-1,1);
    fprintf(stdout,"\nCode attributes:\n");
    if (c & 0x01) fprintf(stdout,"RAMable\n");
    if (c & 0x02) fprintf(stdout,"ROMable\n");
    if (c & 0x04) fprintf(stdout,"Position independent code\n");
    if (c & 0x08) fprintf(stdout,"Mergeable\n");
    if (c & 0x16) fprintf(stdout,"Lex identifier number independent\n");
  }


int main(argc,argv)
int argc;
char **argv;
  {
    int table_address; /* address of lex table */

    SETMODE_STDIN_BINARY;

    if(argc != 1) {
       usage();
       exit(1);
    }

    /* Read in the lex file */
    byte_ptr=0;
    read_file();
    byte_ptr=0;

    /* process lex table */
    lex_table();
    exit(0);
  }
