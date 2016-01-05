/* lexcat71.c -- display the keywords of a HP-71 lex file */
/* 2016 J. Siebold, and placed under the GPL */

/* This program reads an HP-71B lex file (see the HP-71IDS Vol. 1 for the 
   format of this file) from standard input and displays the following
   information on standard output :
 
     LEX file id
     for each of existing lex tables:
       lowest token ID
       highest token ID
       existence of speed table
       existence of message table
       existence of poll handler
       list of keywords with token number and characterization nibble

  Note: the lex file header must not be present
*/

#include<stdio.h>
#include<ctype.h>
#include <stdlib.h>
#include <fcntl.h>


/* This is therefore larger than any real HP41 */
#define LEXFILE_SIZE 131072
#define KEYWORD_LEN 8

/* lex file nibble array */
unsigned char lexfile[LEXFILE_SIZE];
/* actual pointer */
unsigned int  nib_ptr;

void usage(void)
{
   fprintf(stderr,
   "Usage: lexcat71 \n");
   fprintf(stderr,"      (Input comes from standard input)\n");
   fprintf(stderr,"      (Output goes to standard output)(\n");
   fprintf(stderr,"\n");
   exit(1);
}

/*
  read up to LEXFILE_SIZE nibbles into the array lexfile
*/

void read_file(void)
/* Read in the file, return #nibbles stored */
  {
    int c; /* byte read */
    /* Read in the file */
    while((c=getchar())!=EOF) 
      {
        /* convert each byte to nibbles and store them */
        if(nib_ptr+1 > LEXFILE_SIZE)
          {
             fprintf(stderr,"maximum file size exceeded\n");
             exit(1);
          }
        lexfile[nib_ptr]= c & 0xF;
        nib_ptr+=1;
        lexfile[nib_ptr]= (c & 0xF0) >> 4;
        nib_ptr+=1;
      }
    return;
  }

/*
  decode a number from consecutive nibbles with variable length
*/

int read_int(start, len)

int start; /* address of first nibble */
int len;   /* length in nibbles */
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
      retval= retval | (lexfile[start+i] << (i*4));
    }
    return(retval);
  }

/*
  read a keyword from the text table. 
*/
int read_keyword(start,keyword)
int start; /* address of length nibble */
char* keyword; /* keyword returned */

  {
    int i;
    int length; /* length of keyword in characters */
    length= (lexfile[start]+1) /2;
    start++;
    for(i=0;i<length;i++) 
      {
        if(start+i > LEXFILE_SIZE)
          {
           fprintf(stderr,"corrupted file\n");
           exit(1);
          }
         *(keyword+i)= (unsigned char)((lexfile[start+1] << 4) | lexfile[start]);
         start+=2;
      }
    keyword[length]='\0';
    return(start);
  }

/* 
 analyze and dump a lex table. The procedure returns the address of the next
 lex table or 0 otherwise
*/

int lex_table()

  {
    int lexfile_id;      /* id of lex file */
    int lo_tok, hi_tok;  /* lowest an highest token number */
    int next_lextable;   /* offset to next lex table */
    int txt_offset;      /* offset to text table */
    int msg_offset;      /* offset to message table */
    int poll_offset;     /* offset to poll handler */
    int txt_start;       /* start of text table (abs) */
    int key_start;       /* text table entry */
    int token;           /* token */
    int ckr;             /* characterization */
    char keyword[KEYWORD_LEN+1];
    int len;             /* keyword length */

    /* lex file id */
    lexfile_id= read_int(nib_ptr,2);    
    fprintf(stdout,"LEX ID: %x\n",lexfile_id);
    nib_ptr+=2;

    /* Lowest token */
    lo_tok= read_int(nib_ptr,2);    
    fprintf(stdout,"Lowest token: %x\n",lo_tok);
    nib_ptr+=2;

    /* highest token */
    hi_tok= read_int(nib_ptr,2);    
    fprintf(stdout,"Highest token: %x\n",hi_tok);
    nib_ptr+=2;
  
    /* next lextable link */
    next_lextable= read_int(nib_ptr,5);
    if(next_lextable != 0) { 
       fprintf(stdout,"Link to next lex table present\n");
       next_lextable= next_lextable+nib_ptr;
    }
    nib_ptr+=5;

    /* speed table */
    if(read_int(nib_ptr,1)==0xF)
      {
         nib_ptr++;
      }
    else 
      {
        fprintf(stdout,"Speed table present\n");
        nib_ptr+=80;
      }

    /* text table offset */
    txt_offset=read_int(nib_ptr,4);
    txt_start= txt_offset+nib_ptr-1;
    nib_ptr+=4;

    /* message table offset */
    msg_offset=read_int(nib_ptr,4);
    if(msg_offset !=0) fprintf(stdout,"Message table present\n");
    nib_ptr+=4;

    /* poll handler offset */
    poll_offset=read_int(nib_ptr,5);
    if(poll_offset !=0) fprintf(stdout,"Poll handler present\n");
    nib_ptr+=5;

    /* Main table */
    fprintf(stdout," Keyword Token Characterization\n");
    while(1)
      {
         if(nib_ptr == txt_start) break;
         key_start=read_int(nib_ptr,3)+ txt_start;
         nib_ptr+=8;
         len= read_int(key_start,1)+1;
         read_keyword(key_start,keyword);
         token= read_int(key_start+len+1,2);
         ckr=read_int(nib_ptr,1);
         nib_ptr++;
         fprintf(stdout,"%8s %5d ",keyword,token);
         if (ckr == 0xF) fprintf(stdout,"BASIC function\n");
         else 
           {
              int need_comma=0;
              if((ckr & 0x1))  {
                 fprintf(stdout,"Legal from the keyboard");
                 need_comma=1;
              }
              if((ckr & 0x4)) { 
                 if (need_comma) fprintf(stdout,", ");
                 else need_comma=1;
                 fprintf(stdout,"Legal after THEN/ELSE");
              }
              if((ckr & 0x8)) {
                 if (need_comma) fprintf(stdout,", ");
                 fprintf(stdout,"Programmable");
              }
              fprintf(stdout,"\n");
           }
      }
    fprintf(stdout,"\n");
    return(next_lextable);
  }


int main(argc,argv)
int argc;
char **argv;
  {
    int table_address; /* address of lex table */

#ifdef _WIN32
    setmode(fileno(stdin), O_BINARY);
#endif

    if(argc != 1) {
       usage();
       exit(1);
    }

    /* Read in the lex file */
    nib_ptr=0;
    read_file();
    nib_ptr=0;

    /* process lex tables */
    while (1) {
       table_address= lex_table();
       if(table_address== 0) break;
       nib_ptr= table_address;
    }
    exit(0);
  }
