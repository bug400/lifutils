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
#include "lifutils.h"
#include "lif_dir_utils.h"

/* This is therefore larger than any real HP75 */
#define LEXFILE_SIZE 65536

/* Don't know the actual size limit, possible overflow is handled below */
#define KEYWORD_LEN  30


void lexcat75_usage(void)
{
   fprintf(stderr,"Usage : lifutils lexcat75 INPUTFILE [> output file]\n");
   fprintf(stderr,"        or\n");
   fprintf(stderr,"        lifutils lexcat75 < input file [> output file]\n");
   fprintf(stderr,"        Display information about a HP-75 lex file\n");
   fprintf(stderr,"        -r skip an existing LIF header of the input file\n");
   fprintf(stderr,"\n");
}

/*
  read up to LEXFILE_SIZE bytes into the array lexfile
*/

int lexcat75_read_lexfile(FILE * fp,unsigned char * lexfile, unsigned int *lex_file_size)
/* Read in the file */
  {
    int c; /* byte read */

    *lex_file_size=0;

    while((c=getc(fp))!=EOF) 
      {
        /* store bytes */
        if(*lex_file_size+1 > LEXFILE_SIZE)
          {
             fprintf(stderr,"maximum file size exceeded\n");
             return(RETURN_ERROR);
          }
        lexfile[*lex_file_size]= c & 0xFF;
        *lex_file_size+=1;
      }
    return(RETURN_OK);
  }

/*
  decode a number from consecutive bytes with variable length
*/

int lexcat75_read_int(unsigned char *lexfile, unsigned int lex_file_size, int start, int len)

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
      retval= retval | (lexfile[start+i] << (i*8));
    }
    return(retval);
  }
/* 
 analyze and dump a lex table. 
*/

int lexcat75_lex_table(unsigned char * lexfile, unsigned int lex_file_size, int byte_ptr)

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
    int temp;
    unsigned int c;

    /* check type of file in the file directory */
    byte_ptr=5;
    if(lexfile[byte_ptr]!= 0x4C)  {
        fprintf(stderr,"This is not a HP-75 lex file\n");
        return(RETURN_ERROR);
    }
    /* set byte pointer to the beginning of the lex file */
    byte_ptr=18;
    
    /* print the id of the lex file */
    lexfile_id= lexcat75_read_int(lexfile,lex_file_size,byte_ptr,2);    
    return_if_error(lexfile_id);
    fprintf(stdout,"LEX ID: %x (hex)\n",lexfile_id); 

    /* determine offset to the keyword table */
    
    /* set pointer to the beginning of the keyword table */
    temp=lexcat75_read_int(lexfile,lex_file_size,byte_ptr+4,2);
    return_if_error(temp);
    keyword_ptr= byte_ptr+temp;

    /* set pointer to the beginning of the runtime tab */
    temp=lexcat75_read_int(lexfile,lex_file_size,byte_ptr+2,2)+2;
    return_if_error(temp);
    runtime_ptr= byte_ptr+temp;
    fprintf(stdout,"\nKeywords:\n");

    /* now output keywords and their attribute */
    len=0;
    while (lexfile[keyword_ptr] != 0xFF) {

        /* new keyword, the attribute preceeds the function
            code */
        if(len == 0) {
            temp=lexcat75_read_int(lexfile,lex_file_size,runtime_ptr,2);
            return_if_error(temp);
            function_ptr=temp+byte_ptr;
            runtime_ptr+=2;
            temp= lexcat75_read_int(lexfile,lex_file_size,function_ptr-1,1);
            return_if_error(temp);
            attr= temp & 0xC0;
            temp= lexcat75_read_int(lexfile,lex_file_size,function_ptr-1,1);
            return_if_error(temp);
            class= temp & 0x3F;
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
                temp=lexcat75_read_int(lexfile,lex_file_size,sec_attr_ptr,1);
                return_if_error(temp);
                num_params= (temp&0xF0 ) >> 4;
                if(num_params > 0) {
                    fprintf(stdout,"(");
                    shift=2;
                    for(i=0;i<num_params;i++) {
                        temp=lexcat75_read_int(lexfile,lex_file_size,sec_attr_ptr,1); 
                        return_if_error(temp);
                        param_type=((temp & (0x3<<shift))>>shift);
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
                return(RETURN_ERROR);
            } else {
                keyword[len]= (char) c;
                len+=1;
            }
        }
        keyword_ptr+=1;
    }
    fprintf(stdout,"\n");

    /* get the code attributes */
    temp=lexcat75_read_int(lexfile,lex_file_size,byte_ptr+10,2);
    return_if_error(temp);
    c=lexcat75_read_int(lexfile,lex_file_size,temp+byte_ptr-1,1);
    return_if_error((int) c);
    fprintf(stdout,"\nCode attributes:\n");
    if (c & 0x01) fprintf(stdout,"RAMable\n");
    if (c & 0x02) fprintf(stdout,"ROMable\n");
    if (c & 0x04) fprintf(stdout,"Position independent code\n");
    if (c & 0x08) fprintf(stdout,"Mergeable\n");
    if (c & 0x16) fprintf(stdout,"Lex identifier number independent\n");
    return(RETURN_OK);
  }


int lexcat75(int argc,char **argv)
  {

    /* lex file byte array */
    unsigned char lexfile[LEXFILE_SIZE];
    /* actual pointer */
    unsigned int  byte_ptr, lex_file_size;
    /* input file */
    FILE *fp;
    int option, ret;
    int skip_lif=0;

    

    while ((option=getopt(argc,argv,"r?"))!=-1)
      {
        switch(option)
          {
            case 'r':  skip_lif=1;
                       break;
            case '?' : lexcat75_usage();
                       return(RETURN_OK);
          } 
      }
    if((optind!=argc) && (optind!= argc-1))
      {
        lexcat75_usage();
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
       if(skip_lif_header(fp,"LEX75")) {
          fclose(fp);
          fprintf(stderr,"Error: not a LIF HP-75 lex file\n");
          return(RETURN_ERROR);
       } 
    } 

    /* Read in the lex file */
    lex_file_size=0;
    ret=lexcat75_read_lexfile(fp,lexfile,&lex_file_size);
    if(ret== RETURN_ERROR) {
       if(fp!=stdin) fclose(fp);
       return(RETURN_ERROR);
    }
    byte_ptr=0;

    /* process lex table */
    ret=lexcat75_lex_table(lexfile,lex_file_size,byte_ptr);
    if(ret== RETURN_ERROR) {
       if(fp!=stdin) fclose(fp);
       return(RETURN_ERROR);
    }
    return(RETURN_OK);
  }
