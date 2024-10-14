/* decomp41.c -- A filter to print a listing of an HP41 user language (FOCAL) 
   program */
/* 2000 A. R. Duell, and placed under the GPL */

/* An HP41 program consists of a stream of bytes representing the user 
   language instructions. Said instructions are 1-3 bytes long, apart from
   text strings which may take up as many as 16 bytes.

   Further details of the coding of HP41 programs can be found in any book
   on Synthetic Programming, such as 'Extend your HP41' or 'The HP41 
   Synthetic Quick Reference Guide */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include "config.h"
#include"byte_tables41.h"
#include "xrom.h"

/* MEMORY_SIZE is set larger than the program memory of any real HP41 so that
   any program can be read into a suitable array of this size */

#define MEMORY_SIZE 4096

void check_length(int *pc, int length, int num_bytes)
{
   if(*pc+num_bytes-1 >= length) {
      fprintf(stderr,"premature end of bytecode\n");
      exit(1);
   }
}

int read_prog(unsigned char *memory)
  {
    int byte_counter;
    int byte;

    byte_counter=0; /* Start loading at start of memory */
    while(((byte=getchar())!=EOF) && (byte_counter<MEMORY_SIZE))
      {
        memory[byte_counter]=byte;
        byte_counter++;
      }
    return(byte_counter); /* number of bytes read */
  }

void print_hex(unsigned char *memory, int pc, int count)
  {
    /* Print count bytes of program memory */
    int i; /* byte counter */
    /* print the address */
    printf("** %04X :",pc);
    /* print the bytes */
    for(i=0; i<count; i++)
      {
        printf(" %02X",memory[pc+i]);
      }
    printf("\n");
  }

void print_suffix(unsigned char byte)
  {
    /* Print the decoded suffix for most 2-byte instructions */
    /* If the high bit is set, it's INDirect */
    if(byte&0x80)
      {
        printf(" IND");
      }
    /* If the low 7 bits are <102, it's a simple numeric suffix. Otherwise
       it's an alpha one */
    if((byte&0x7f)<102)
      {
        printf(" %02d\n",byte&0x7f);
      }
    else
      {
        printf(" %c\n",suffix[(byte&0x7f)-102]);
      }
  }

void print_string(unsigned char *memory, int pc, int length, int alt_flag)
  {
    /* Print a string of length characters from memory, starting at pc */
    int i; /* character counter */
    unsigned char c;

    /* Handle append char : "~String" */
    if(memory[pc]== 0x7f) 
    {
        if(alt_flag) 
        {
           printf("\"");
           fputs("├",stdout);
        } else
        {
           putchar('>');
           printf("\"");
        }
        pc++;
        length--;
    } 
    else
    {
       printf("\"");
       if(length>1 && memory[pc]=='>' && memory[pc+1]=='-') 
       {
          printf("\\x%02X",memory[pc]);        
          printf("\\x%02X",memory[pc+1]);        
          pc+=2;
          length-=2;
       } 
       else if (length>1 && memory[pc]=='-' && memory[pc+1]=='>')
       {
          printf("\\x%02X",memory[pc]);        
          printf("\\x%02X",memory[pc+1]);        
          pc+=2;
          length-=2;
       }
       else if (length>1 && memory[pc]=='\\' && memory[pc+1]=='-')
       {
          printf("\\x%02X",memory[pc]);        
          printf("\\x%02X",memory[pc+1]);        
          pc+=2;
          length-=2;
       }
       else if (length>1 && memory[pc]=='|' && memory[pc+1]=='-')
       {
          printf("\\x%02X",memory[pc]);        
          printf("\\x%02X",memory[pc+1]);        
          pc+=2;
          length-=2;
       }
       else if (length>0 && memory[pc]=='>')
       {
          printf("\\x%02X",memory[pc]);        
          pc++;
          length--;
       }
       else if (length>0 && memory[pc]=='~')
       {
          printf("\\x%02X",memory[pc]);        
          pc++;
          length--;
       }
    }
    printf("%s\"\n",to_hp41_string(memory+pc,length,alt_flag));
  }

void xrom(unsigned char *memory, int pc, int line, int line_flag, int alt_flag)
  {
    /* Print the appropriate name from the xrom_names[][] array */
    int rom; /* The ROM select code */
    int fn; /* The function in that ROM */
    int ind; /* array index in xrom table */

    rom = (memory[pc]&7)*4 + (memory[pc+1]>>6);
    fn=memory[pc+1]&0x3f;
    ind=find_xrom_by_id(rom,fn);
    if(line_flag) printf("%04d  ",line);
    if (ind == -1 ) 
    {
       printf("XROM %02d,%02d\n",rom,fn);
    }
    else
       printf("%s\n",(alt_flag ? get_xrom_alt_name_by_index(ind): get_xrom_name_by_index(ind)));
  }

void short_lbl(unsigned char *memory, int length, int *pc, int *line, int hex_flag, int line_flag)
  {
    check_length(pc,length,1);

    /* Print the short-form labels (and handle NULL) from row 0 of 
       the byte table */
    if(hex_flag)
      {
        print_hex(memory, *pc, 1);
      }
    /* If this is not NULL, print the line */
    if(memory[*pc])
      {
        /* print the instuction and inc line */
        if(line_flag) printf("%04d  ",*line);
        printf("LBL %02d\n",memory[*pc]-1);
        (*line)++;
      }
    (*pc)++; /* inc program counter */
  }

void short_sto_rcl(unsigned char *memory, int length, int *pc, int *line, int hex_flag, int line_flag)
  {
    check_length(pc,length,1);

    /* Print the short-form STO and RCL instructions from rows 2-3 of 
       the byte table */
    if(hex_flag)
      {
        print_hex(memory,*pc,1);
      }
    /* Print the instruction */
    if(line_flag) printf("%04d  ",*line);
    printf("%s %02d\n",((memory[*pc]>>4)==2)?"RCL":"STO",
           memory[*pc]&0xf);
    (*line)++; /* increment line */
    (*pc)++; /* and pc */
  }

void short_gto(unsigned char *memory, int length, int *pc, int *line, int hex_flag, int line_flag)
  {
    check_length(pc,length,2);

    /* Print the short-form GTOs (and handle NULL) from row B of 
       the byte table */
    if(hex_flag)
      {
        print_hex(memory, *pc, 2);
      }
    /* If this is not NULL, print the line */
    if(memory[*pc]!=0xb0)
      {
        /* print the instuction and inc line */
        if(line_flag) printf("%04d  ",*line);
        printf("GTO %02d\n",(memory[*pc]&0xf)-1);
        (*line)++;
      }
    (*pc)+=2; /* inc program counter */
  }

void swap_lbl(unsigned char *memory, int length, int *pc, int *line, int hex_flag, int line_flag)
  {
    check_length(pc,length,2);

    /* Print the odd 2-byte instructions at the end of row C of 
       the byte table */
    if(hex_flag)
      {
        print_hex(memory,*pc,2);
      }
    /* Print the instruction */
    if(line_flag) printf("%04d  ",*line);
    printf("%s",row_c[memory[*pc]-0xce]);
    print_suffix(memory[(*pc)+1]);
    (*line)++; /* Incrememnt line number */
    (*pc)+=2; /* And pc */
  }

int global_end(unsigned char *memory, int length, int *pc, int *line, int hex_flag, int line_flag, int alt_flag)
  {
    /* Print global labels and ENDs from the start of row C of the
       byte table */
    /* If the high bit of the 3rd byte of the instuction is set, then 
       it's a global label, otherwise it's an END */
    int end_flag;

    check_length(pc,length,2);
    check_length(pc,length,(memory[(*pc)+2]&0x80)?(memory[(*pc)+2]&0xf)+3:3);

    if(hex_flag)
      {
        print_hex(memory,*pc,
        (memory[(*pc)+2]&0x80)?(memory[(*pc)+2]&0xf)+3:3);
      } 
    if(memory[(*pc)+2]&0x80)
      {
        /* It's a label */
        if(line_flag) printf("%04d  ",*line);
        printf("LBL ");
        /* Miss off the first character of the string (key asignment) */
        print_string(memory,(*pc)+4,(memory[(*pc)+2]&0xf)-1,alt_flag);
        end_flag=0;
      }
    else
      {
        /* It's an END */
        /* Bit 5 of the third byte distinguishes a local end from a global
           one */
        check_length(pc,length,2);
        if(line_flag) printf("%04d  ",*line);
        printf("%s\n",(memory[(*pc)+2]&0x20)?".END.":"END");
        end_flag=1;
      }
    (*line)++; /* Incrememnt line number */
    (*pc)+=(memory[(*pc)+2]&0x80)?(memory[(*pc)+2]&0xf)+3:3;
    return(end_flag); /* Is this the end of the program? */
  }

void print_gto_xeq(unsigned char *memory, int length, int *pc, int *line, int hex_flag, int line_flag)
  {
    check_length(pc,length,3);

    /* Print the 3-byte XEQ and GTO instructions from rows D--E of
       the byte table */
    if(hex_flag)
      {
        print_hex(memory,*pc,3);
       }
    /* print the instruction */
    if(line_flag) printf("%04d  ",*line);
    printf("%s",((memory[*pc]>>4)==0xd)?"GTO":"XEQ");
    print_suffix(memory[(*pc)+2]&0x7f);
    (*line)++; /* increment line number */
    (*pc)+=3; /* This is a 3 byte instruction */
  }

void print_alpha_gto(unsigned char *memory, int length, int *pc, int *line, int hex_flag, int line_flag, int alt_flag)
  {
    check_length(pc,length,1);
    check_length(pc,length,(memory[(*pc)+1]&0xf)+2);

    /* Print GTOs and XEQs with alpha destinations */
    if(hex_flag)
      {
        print_hex(memory,*pc,(memory[(*pc)+1]&0xf)+2);
      }
    /* print the instruction */
    if(line_flag) printf("%04d  ",*line);
    printf("%s ",alpha_gto[memory[*pc]-0x1d]);
    print_string(memory,(*pc)+2,memory[(*pc)+1]&0xf, alt_flag);
    (*line)++; /* Increment line number */
    /* 'Extend Your HP41' section 15.7 says that if the high nybble of 
       the second byte is not f, then the pc is only incremented by 2 */
    (*pc)+=((memory[(*pc)+1]>>4)==0xf)?(memory[(*pc)+1]&0xf)+2:2;
  }

void print_digits(unsigned char *memory, int length, int *pc, int *line, int hex_flag, int line_flag)
  {
    /* Print numbers in program listings */
    /* This is the only function where the number of bytes can't be 
       deduced from the opcode/suffix. Instead, keep on printing until
       the next byte is not numeric */
    int temp_pc; /* temporary pc for printing in hex */
    if(hex_flag)
      {
        temp_pc=(*pc);
        printf("** %04X :",temp_pc); /* print the address */
        do
          {
            printf(" %02X",memory[temp_pc]);
            check_length(&temp_pc,length,1);
            temp_pc++;
          }
        while((memory[temp_pc]>=0x10) && (memory[temp_pc]<=0x1c));
        printf("\n");
      }
    /* Print the number */
    if(line_flag) printf("%04d  ",*line);
    do
      {
        /* if(memory[*pc] == 0x1b) putchar(' '); */
        putchar(digit[memory[(*pc)]-0x10]); /* print next digit */
        (*pc)++;
      }
    while((memory[*pc]>=0x10) && (memory[*pc]<=0x1c));
    printf("\n");
    (*line)++;
  }
     
void print_1_byte(unsigned char *memory, int length, int *pc, int *line, int hex_flag, int line_flag, int alt_flag)
  {
    /* Print the single-byte instructions from rows 4--8 of the byte 
       table */ 
 
    check_length(pc,length,1);

    if(hex_flag)
      {
        print_hex(memory,*pc,1);
      }
    /* print the program line */
    if(line_flag) printf("%04d  ",*line);
    printf("%s\n",(alt_flag) ? alt_single_byte[memory[*pc]-0x40]: single_byte[memory[*pc]-0x40]);
    (*line)++; /* Increase the line counter */
    (*pc)++; /* And the program counter */
  }

void print_2_byte(unsigned char *memory, int length, int *pc, int *line, int hex_flag, int line_flag, int alt_flag)
  {
    check_length(pc,length,2);

    /* Print the double-byte instructions from rows 9--A of the byte table */
    if(hex_flag)
      {
        print_hex(memory, *pc, 2);
      }
    /* Handle the special cases */
    /* Skip the unused 2-byte instruction 0xaf */
    if(memory[*pc]!=0xaf)
      {
        if(memory[*pc]==0xae)
          {
            /* It's a GTO or XEQ indirect */
            if(line_flag) printf("%04d  ",*line);
            printf("%s",(memory[(*pc)+1]&0x80)?"XEQ":"GTO");
            print_suffix((memory[(*pc)+1]&0x7f)+0x80);
          }
        else
          {
            /* Is it an XROM ? */
            if((memory[*pc]>=0xa0) && (memory[*pc]<=0xa7))
              {
                xrom(memory,*pc,*line,line_flag,alt_flag);
              }
            else
              {
                if(line_flag) printf("%04d  ",*line);
                printf("%s",(alt_flag) ? alt_double_byte[memory[*pc]-0x90]: double_byte[memory[*pc]-0x90]);
                print_suffix(memory[(*pc)+1]);
              }
          }
        (*line)++; /* If anything was printed, increment the line */
      }
   (*pc)+=2; /* Move pc on 2 bytes */
  }

void print_row_1(unsigned char *memory, int length, int *pc, int *line, int hex_flag, int line_flag, int alt_flag)
  {
    /* Print everything in row 1 of the byte table */
    if(memory[*pc]>=0x1d)
      {
        /* It's an alpha GTO/XEQ */
        print_alpha_gto(memory,length,pc,line,hex_flag,line_flag, alt_flag);
      }
    else
      {
        print_digits(memory,length,pc,line,hex_flag,line_flag);
      }
  }
int print_row_c(unsigned char *memory, int length, int *pc, int *line, int hex_flag, int line_flag, int alt_flag)
  {
    /* Print everything in row c of the byte table */
    if(memory[*pc]>=0xce)
      {
        /* It's one of the odd 2 instructions */
        swap_lbl(memory,length,pc,line,hex_flag,line_flag);
        return(0); /* This can't be the end */
      }
    else
      {
        /* It's a global label or an end */
        return(global_end(memory,length, pc,line,hex_flag, line_flag, alt_flag));
      }
  }

void print_text(unsigned char *memory, int length, int *pc, int *line, int hex_flag,int line_flag, int alt_flag)
  {
    check_length(pc,length,(memory[*pc]&0xf)+1);
    /* Print text strings started by bytes in row F of the byte table */
    if(hex_flag)
      {
        /* low nybble of the instruction byte is the length */
        print_hex(memory,*pc,(memory[*pc]&0xf)+1);
      }
    if(line_flag) printf("%04d  ",*line);
    print_string(memory,(*pc)+1, memory[*pc]&0xf,alt_flag); /* And string */
    (*line)++; /* Increment line number */ 
    (*pc)+=(memory[*pc]&0xf)+1; /* And pc to after the string */
  }

int print_instruction(unsigned char *memory, int length, int *pc, int *line, int hex_flag, int line_flag, int alt_flag)
  {
    /* Instructions with the same high nybble tend to have similar
       formats, so decode based on this */
    int end_flag; /* Set to signal end of program */
    end_flag=0;
    switch(memory[*pc]>>4)
      {
        case 0: short_lbl(memory,length,pc,line,hex_flag,line_flag);
                break;
        case 1: print_row_1(memory,length,pc,line,hex_flag,line_flag, alt_flag);
                break;
        case 2:
        case 3: short_sto_rcl(memory,length,pc,line,hex_flag,line_flag);
                break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8: print_1_byte(memory,length,pc,line,hex_flag,line_flag,alt_flag);
                break;
        case 9:
        case 0xa: print_2_byte(memory,length,pc,line,hex_flag,line_flag,alt_flag);
                  break;
        case 0xb: short_gto(memory,length,pc,line,hex_flag,line_flag);
                  break;
        case 0xc: end_flag=print_row_c(memory,length,pc,line,hex_flag, line_flag,alt_flag)?1:end_flag;
                  break; 
        case 0xd:
        case 0xe: print_gto_xeq(memory,length,pc,line,hex_flag,line_flag);
                  break;
        case 0xf: print_text(memory,length,pc,line,hex_flag,line_flag,alt_flag);
                  break;
      }
    return(end_flag); 
  }
 
void list_prog(unsigned char *memory, int length, int hex_flag, int line_flag, int alt_flag)
  {
    int pc; /* current program counter */
    int end_flag=0; /* End of program detected */
    int line; /* line number of user program */
    /* Scan throguh the loaded program and print the instructions */
    pc=0;
    line=1;
    while ((pc<length) && (!end_flag))
      {
        end_flag=print_instruction(memory,length,&pc,&line,hex_flag,line_flag, alt_flag);
      }
  }

void usage(void)
  {
    fprintf(stderr,"Usage: decomp41 [-h] [-x xrom_name_file][-x...]\n");
    fprintf(stderr,"       -h flag prints instruction bytes in hex before each program line\n");
    fprintf(stderr,"       -x xrom_name_file uses those names for XROM functions\n");
    fprintf(stderr,"       -a flag prints function names with UTF-8 characters\n");
    fprintf(stderr,"       -l add line numbers\n");
    exit(1);
  }

int main(int argc, char **argv)
  {
    int option; /* current option character */
    int prog_length; /* program length */
    int hex_flag=0; /* Print bytes in hex first? */
    int line_flag=0; /* Print line numbers) */
    int alt_flag=0;  /* alternate UTF-8 function names */
    unsigned char memory[MEMORY_SIZE]; /* HP41 program memory */

    init_xrom(); /* Initialize xrom */

    SETMODE_STDIN_BINARY;

    optind=1;
    while((option=getopt(argc,argv,"ahlx:?"))!=-1)
      {
        switch(option)
          {
            case 'h' : hex_flag=1;
                       break;
            case 'x' : read_xrom(optarg);
                       break;
            case 'l' : line_flag=1;
                       break;
            case 'a' : alt_flag=1;
                       break;
            case '?' : usage();
           }
      }
    if(optind!=argc)
      {
        usage();
      }

    prog_length=read_prog(memory); /* Read in the program */
    list_prog(memory,prog_length,hex_flag, line_flag,alt_flag); /* list it */
    exit(0);
  }
