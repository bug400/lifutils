/* lexcat71.c -- display the keywords of a HP-71 lex file */
/* 2016-2020 J. Siebold, and placed under the GPL */

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

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"
#include "lifutils.h"
#include "lif_dir_utils.h"

/* This is therefore larger than any real HP41 */
#define LEXFILE_SIZE 131072
#define KEYWORD_LEN 8


void lexcat71_usage(void)
{
   fprintf(stderr,"Usage : lifutils lexcat71 [-r] INPUTFILE [> output file]\n");
   fprintf(stderr,"        or\n");
   fprintf(stderr,"        lifutils lexcat71 [-r] < input file [> output file]\n");
   fprintf(stderr,"        Display main and text table information of a HP-71 lex file\n");
   fprintf(stderr,"        -r skip an existing LIF header of the input file\n");
   fprintf(stderr,"\n");
}

/*
  read up to LEXFILE_SIZE nibbles into the array lexfile
*/

int lexcat71_read_lexfile(FILE* fp, unsigned char *lexfile, unsigned int *lex_file_size)
/* Read in the file */
  {
    int c; /* byte read */

    /* Read in the file */
    *lex_file_size=0;
    while((c=getc(fp))!=EOF) 
      {
        /* convert each byte to nibbles and store them */
        if(*lex_file_size+1 > LEXFILE_SIZE)
          {
             fprintf(stderr,"maximum file size exceeded\n");
             return(RETURN_ERROR);
          }
        lexfile[*lex_file_size]= c & 0xF;
        *lex_file_size+=1;
        lexfile[*lex_file_size]= (c & 0xF0) >> 4;
        *lex_file_size+=1;
      }
    return(RETURN_OK);
  }

/*
  decode a number from consecutive nibbles with variable length
*/

int lexcat71_read_int(unsigned char *lexfile, unsigned int lex_file_size, int start, int len)

  {
    int retval;
    int i;

    retval=0;
    for(i=0;i<len;i++) {
      if((unsigned int)(start+i) > lex_file_size)
        {
           fprintf(stderr,"corrupted file\n");
           return(RETURN_ERROR);
        }
      retval= retval | (lexfile[start+i] << (i*4));
    }
    return(retval);
  }

/*
  read a keyword from the text table. 
*/
int lexcat71_read_keyword(unsigned char *lexfile, unsigned int lex_file_size, int start, char *keyword)

  {
    int i;
    int length; /* length of keyword in characters */
    length= (lexfile[start]+1) /2;
    start++;
    for(i=0;i<length;i++) 
      {
        if((unsigned int)(start+i) > lex_file_size)
          {
           fprintf(stderr,"corrupted file\n");
           return(RETURN_ERROR);
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

int lexcat71_lex_table(unsigned char *lexfile, unsigned int lex_file_size, int nib_ptr)

  {
    int lexfile_id;      /* id of lex file */
    int lo_tok, hi_tok;  /* lowest an highest token number */
    int next_lextable;   /* offset to next lex table */
    int txt_offset;      /* offset to text table */
    int msg_offset;      /* offset to message table */
    int speed_offset;     /* offset to speed table */
    int poll_offset;     /* offset to poll handler */
    int txt_start;       /* start of text table (abs) */
    int key_start;       /* text table entry */
    int token;           /* token */
    int ckr;             /* characterization */
    char keyword[KEYWORD_LEN+1];
    int len;             /* keyword length */

    /* lex file id */
    lexfile_id= lexcat71_read_int(lexfile,lex_file_size,nib_ptr,2);    
    return_if_error(lexfile_id);
    fprintf(stdout,"LEX ID: %x\n",lexfile_id);
    nib_ptr+=2;

    /* Lowest token */
    lo_tok= lexcat71_read_int(lexfile,lex_file_size,nib_ptr,2);    
    return_if_error(lo_tok);
    fprintf(stdout,"Lowest token: %x\n",lo_tok);
    nib_ptr+=2;

    /* highest token */
    hi_tok= lexcat71_read_int(lexfile,lex_file_size,nib_ptr,2);    
    return_if_error(hi_tok);
    fprintf(stdout,"Highest token: %x\n",hi_tok);
    nib_ptr+=2;
  
    /* next lextable link */
    next_lextable= lexcat71_read_int(lexfile,lex_file_size,nib_ptr,5);
    return_if_error(next_lextable);
    if(next_lextable != 0) { 
       fprintf(stdout,"Link to next lex table present\n");
       next_lextable= next_lextable+nib_ptr;
    }
    nib_ptr+=5;

    /* speed table */
    speed_offset=lexcat71_read_int(lexfile,lex_file_size,nib_ptr,1);
    return_if_error(speed_offset);
    if(speed_offset==0xF)
      {
         nib_ptr++;
      }
    else 
      {
        fprintf(stdout,"Speed table present\n");
        nib_ptr+=80;
      }

    /* text table offset */
    txt_offset=lexcat71_read_int(lexfile,lex_file_size,nib_ptr,4);
    return_if_error(txt_offset);
    txt_start= txt_offset+nib_ptr-1;
    nib_ptr+=4;

    /* message table offset */
    msg_offset=lexcat71_read_int(lexfile,lex_file_size,nib_ptr,4);
    return_if_error(msg_offset);
    if(msg_offset !=0) fprintf(stdout,"Message table present\n");
    nib_ptr+=4;

    /* poll handler offset */
    poll_offset=lexcat71_read_int(lexfile,lex_file_size,nib_ptr,5);
    return_if_error(poll_offset);
    if(poll_offset !=0) fprintf(stdout,"Poll handler present\n");
    nib_ptr+=5;

    /* Main table */
    fprintf(stdout," Keyword Token Characterization\n");
    while(1)
      {
         if(nib_ptr == txt_start) break;
         key_start=lexcat71_read_int(lexfile,lex_file_size,nib_ptr,3)+ txt_start;
         return_if_error(key_start);
         nib_ptr+=8;
         len= lexcat71_read_int(lexfile,lex_file_size,key_start,1)+1;
         return_if_error(len);
         if(lexcat71_read_keyword(lexfile,lex_file_size,key_start,keyword)== RETURN_ERROR) return(RETURN_ERROR);
         token= lexcat71_read_int(lexfile,lex_file_size,key_start+len+1,2);
         return_if_error(token);
         ckr=lexcat71_read_int(lexfile,lex_file_size,nib_ptr,1);
         return_if_error(ckr);
    
         nib_ptr++;
         
         /* do not output entries, which begin with lowercase letters */
         if(keyword[0]>= 'a' && keyword[0] <= 'z') continue;

         fprintf(stdout,"%8s %5d ",keyword,token);
         if (ckr == 0xF) fprintf(stdout,"BASIC function\n");
         else if (ckr ==0) fprintf(stdout,"Intermediate\n");
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


int lexcat71(int argc, char **argv)
  {
    int table_address; /* address of lex table */
    /* lex file nibble array */
    unsigned char lexfile[LEXFILE_SIZE];
    /* actual pointer */
    unsigned int  nib_ptr;
    /* lex file size */
    unsigned int lex_file_size;
    /* input file */
    FILE *fp;
   
    int option,ret;
    int skip_lif=0;


    while ((option=getopt(argc,argv,"r?"))!=-1)
      {
        switch(option)
          {
            case 'r' : skip_lif=1;
                       break;
            case '?' : lexcat71_usage();
                       return(RETURN_OK);
          }
      }
    if((optind!=argc) && (optind!= argc-1))
      {
        lexcat71_usage();
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
    if(skip_lif) {
       if(skip_lif_header(fp,"LEX71")) {
          fclose(fp);
          fprintf(stderr,"Error: not a LIF HP-71 lex file\n");
          return(RETURN_ERROR);
       } 
    } 
    /* Read in the lex file */
    ret=lexcat71_read_lexfile(fp,lexfile,&lex_file_size);
    if(ret== RETURN_ERROR) {
       if(fp!=stdin)fclose(fp);
       return(RETURN_ERROR);
    }
    nib_ptr=0;

    /* process lex tables */
    while (1) {
       table_address= lexcat71_lex_table(lexfile,lex_file_size,nib_ptr);
       if(table_address== RETURN_ERROR) {
          if(fp!=stdin)fclose(fp);
          return(RETURN_ERROR);
       }
       if(table_address== 0) break;
       nib_ptr= table_address;
    }
    if(fp!=stdin)fclose(fp);
    return(RETURN_OK);
  }
