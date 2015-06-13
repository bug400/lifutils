/* wall41.c -- Extract information from an HP41 Write-All file */
/* 2000 A. R. Duell, and placed under the GPL */

/* An HP41 Write-All (Wall) file contains a simple dump of the user 
   memory in the obvious order. Extended memory is not included. Each 8 
   byte 'record' of the file corresponds to a 7-byte HP41 'register' 
   using the standard nybble scrambling. 

   The first 16 records of the file contain the machine stack and status
   registers. The 'missing' registers 0x10 to 0xbf are not stored. The   
   remaining records of the file contain (in approximately this order) 
   the Key Assignment Registers (KARs), buffers, user programs and user 
   data registers.

   This program extracts the various items from a write-all file and  
   outputs them in the standard format. Programs are turned into streams 
   of bytes, other information is re-scrambled from 7 byte registers to 
   8 byte records so that the programs sdata, key41, regs41 and so on can 
   read it */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <fcntl.h>
#include"descramble_41.h"
#include"scramble_41.h"

/* A structure type to store parameters for output of a particular section*/ 
typedef struct 
  {
    char name[80]; /* output filename */
    unsigned char flag; /* This section is to be output */
  } outtype;

/* sizes of file record and HP41 register */
#define RECORD_LENGTH 8
#define REGISTER_LENGTH 7
/* smallest and largest byte counts (after conversion to registers */
#define HP41C_SIZE 560
#define HP41CV_SIZE 2352

/* This is larger than any real HP41's memory */
#define MEMORY_SIZE 4096

/* HP41 memory */
unsigned char memory[MEMORY_SIZE];

int correct_for_gap(int value)
/* Correct for the 'missing registers' not saved in a Write-All file */
  {
    if(value>0xf)
      {
        value-=176;
      }
    return(value);
  }

int read_file(void)
/* Read in the Write-All file. Return the number of bytes read */
  {
    unsigned char rec[RECORD_LENGTH]; /* Current file record */
    int byte_pointer=0;
    /* Read the file until end */
    while((fread(rec,sizeof(unsigned char),RECORD_LENGTH,stdin)==RECORD_LENGTH)
           && (byte_pointer<MEMORY_SIZE))
      {
        /* convert each record to a register and store it in 'memory' */
        descramble(rec,memory+byte_pointer);
        byte_pointer+=7;
      }
    return(byte_pointer); /* Return #bytes read */
  }

void output_status(char *filename,int size, int sreg)
/* Output the status information as a stat41 file */
  {
    int byte_counter;
    unsigned char rec[RECORD_LENGTH]; /* file record */
    unsigned char tmp_reg[REGISTER_LENGTH]; /* 'register' to be used to build 
                                                up the last 2 records in 
                                                the file */

    FILE *stat_file;
    /* If necessary, open the file */
    if(strlen(filename))
      {
        stat_file=fopen(filename,"wb");
      }
    else
      {
        stat_file=stdout;
      }

    /* The first 8 records in the file are the first 8 registers 
       of the HP41 memory, unchanged */
    for(byte_counter=0; byte_counter<56; byte_counter+=7)
      {
        /* scramble and output the record */
        scramble(memory+byte_counter,rec);
        fwrite(rec,sizeof(unsigned char),RECORD_LENGTH,stat_file);
      }

    /* create 'register 8' -- rest of alpha register, size, sreg */
    memcpy(tmp_reg,memory+56,7); /* copy leftmost bit of alpha */
    tmp_reg[0]=size>>4;
    tmp_reg[1]=((size&0xf)<<4)+(sreg>>8);
    tmp_reg[2]=(sreg&0xff);
    /* and output it */
    scramble(tmp_reg,rec);
    fwrite(rec,sizeof(unsigned char),RECORD_LENGTH,stat_file);

    /* create 'register 9' -- first 44 user flags */
    memcpy(tmp_reg,memory+98,7);
    tmp_reg[5]&=0xf0; /* Mask out other flags */
    tmp_reg[6]=0;
    /* output it */
    scramble(tmp_reg,rec);
    fwrite(rec,sizeof(unsigned char),RECORD_LENGTH,stat_file);

    /* If a file was opened, close it */
    if(strlen(filename))
      {
        fclose(stat_file);
      }
  }

void output_keys_buffs(outtype keys, outtype *buffers, int global_end)
/* If required, output the KARs (as a keys41 file) and buffers (as a HP41
   register file). These are grouped together because the only way to 
   find the first buffer is by scanning over the KARs */
  {
    FILE *keyfile; /* Output filr for keys */
    FILE *bufferfile; /* Output file for buffers */
    int byte_counter; /* Address in HP41 'memory' */
    int buffno; /* Current buffer number */
    int bufflen; /* and length in registers */
    int buffreg; /* Current register in buffer */
    unsigned char rec[RECORD_LENGTH]; /* Record in output files */

    /* If a key file name is given, open the file, else set it to stdout */
    if(strlen(keys.name))
      {
        keyfile=fopen(keys.name,"wb");
      }
    else
      {
        keyfile=stdout;
      }

    byte_counter=16*7; /* Start in register 16 (which is actually HP41
                          register 0xc0 */

    /* Scan over the KARs, each one starts with 0xf0 */
    while((byte_counter<global_end*7) && (memory[byte_counter]==0xf0))   
      {
        /*If key definitions are to be output, do so */
        if(keys.flag)
          {
            scramble(memory+byte_counter,rec);
            fwrite(rec,sizeof(unsigned char),8,keyfile);
          }
        byte_counter+=7; /* Point to next register */
      }

    /* If a file was opened, close it */
    if(strlen(keys.name))
      {
        fclose(keyfile);
      }

    /* All the KARs have been scanned, and byte_counter now points to
       the first buffer (if present). Now scan the buffers. The first 
       byte of a buffer is a multiple of 0x11, and is non-zero */

    while((byte_counter<global_end*7) && memory[byte_counter] &&
          !(memory[byte_counter] % 0x11))
      {
        buffno=memory[byte_counter]&0xf; /* Either nybble gives the
                                            buffer ID */
        bufflen=memory[byte_counter+1]; /*length in registers */
        /*If there is a filename for this buffer, open the file */
        if(strlen((buffers+buffno)->name))
          {
            bufferfile=fopen((buffers+buffno)->name,"wb");
          }
        else
          {
            bufferfile=stdout;
          }
        /* Now scan over the buffer */
        for(buffreg=0; buffreg<bufflen; buffreg++) 
          {
            /* If this buffer is to be output, do so */
            if((buffers+buffno)->flag)
              {
                scramble(memory+byte_counter,rec);
                fwrite(rec,sizeof(unsigned char),RECORD_LENGTH,bufferfile);
              }
            byte_counter+=7; /* Point to next register */
          }

        /* If a file was opened, close it */
        if(strlen((buffers+buffno)->name))
          {
            fclose(bufferfile);
          }
      }
  }

void inc_pc(int *reg, int *byte_offset)
/* Increment the pointer to a byte of an HP41 program (represented as a 
   register and a byte offset in that register to point to the next byte.
   Remember that HP41 programs are stored backwards */
  {
    (*byte_offset)++;
    if((*byte_offset)>6)
      {
        (*reg)--;
        (*byte_offset)=0;
      }
  }

unsigned char copy_byte(FILE *progfile, int *reg, int *byte_offset)
/* Copy one byte of an HP41 program from 'memory' to the output file. 
   Increment the pointer and return the byte moved */
  {
    unsigned char prog_byte;

    /* Read a byte and output it */
    prog_byte=memory[(*reg)*7+(*byte_offset)];
    fputc(prog_byte,progfile);

    /* Increment address */
    inc_pc(reg,byte_offset);

    return(prog_byte);
  }

void copy_bytes(FILE *progfile, int *reg, int *byte_offset, int n)
/* Copy n bytes of an HP41 program to the output file */
  {
    int i; /* Byte counter */

    for(i=0; i<n; i++)
      {
        copy_byte(progfile, reg, byte_offset);
      }
  }

FILE *open_prog(char *prog_name, int prog_no)
/* Open program output file, of form progname.00n */
  {
    char name[84];

    sprintf(name,"%s.%03d",prog_name,prog_no);
    return(fopen(name,"wb"));

  }

void output_progs(char *prog_name, int curtain, int global_end)
/* Output HP41 programs to named files */
  {
    int prog_no=0; /* Current program number */
    int reg, byte_offset; /* Address pointer in program memory */
    unsigned char prog_byte; /* Current prefix byte */
    unsigned char global_flag=0; /* global end found */
    FILE *prog_file;

    reg=curtain-1; byte_offset=0; /* Initialise address pointer */
    
    /* Open first output file */
    prog_file=open_prog(prog_name,prog_no);

    /* Scan through program memory */
    while((!global_flag) && (reg>=global_end))
      {
        /* Copy a byte and see if there's more to be done for this
           instruction */
        switch((prog_byte=copy_byte(prog_file,&reg,&byte_offset))>>4)
          {
            /* bytes 0x00-0x8f are single-byte instructions, so
               no more to do. Note that alpha XEQ/GTO are handled as 
               a single-byte instruction follwed by a string */
            case 0x9 :
            case 0xa :
            case 0xb : /* 2 byte instructions -- copy another byte */
                       copy_byte(prog_file,&reg,&byte_offset);
                       break;
            case 0xc : /* labels and a couple of leftovers */
                       copy_byte(prog_file,&reg,&byte_offset);
                       if(prog_byte<=0xcd)
                         {
                           /* It's a label or end */
                           prog_byte=copy_byte(prog_file,&reg,&byte_offset);
                           if(prog_byte&0x80)
                             {
                               /* If high bit set, it's a label */
                               copy_bytes(prog_file,&reg,&byte_offset,
                                          prog_byte&0xf);
                             }
                           else
                             {
                               /* It's an end */
                               if(prog_byte&0x20)
                                 {
                                   /* It's the global end */
                                   global_flag=1;
                                 }
                               else
                                 {
                                   /* Start a new program file */
                                   fclose(prog_file);
                                   prog_no++;
                                   prog_file=open_prog(prog_name,prog_no);
                                 }
                             }
                         }
                       break;  
            case 0xd :
            case 0xe : /* 3 byte instructions -- copy 2 more bytes */
                       copy_byte(prog_file,&reg,&byte_offset);
                       copy_byte(prog_file,&reg,&byte_offset);
                       break;
            case 0xf : /* Strings */
                       copy_bytes(prog_file,&reg,&byte_offset,prog_byte&0xf);
                       break;
          }
      }
    /* close last file */
    fclose(prog_file); 
  }

void output_registers(char *filename, int curtain, int bytes_read)
/* Output the user data registers as an sdata file */
  {
    FILE *reg_file;
    int byte_counter; 
    unsigned char rec[RECORD_LENGTH]; /* output file record */

    /* If necessary, open the output file */
    if(strlen(filename))
      {
        reg_file=fopen(filename,"wb");
      }
    else
      {
        reg_file=stdout;
      }

    /* Now output the data from the curtain to the top of memory */
    for(byte_counter=curtain*7; byte_counter<bytes_read; byte_counter+=7)
      {
        /* scramble and output each register */ 
        scramble(memory+byte_counter,rec);
        fwrite(rec,sizeof(unsigned char),RECORD_LENGTH,reg_file);
      }
    /* If a file was opened, close it */
    if(strlen(filename))
      {
        fclose(reg_file);
      }
  }

void usage(void)
  {
    fprintf(stderr,"Usage : wall41 [-k [keyfile]] [-b buff# [buff_file]]\n");
    fprintf(stderr,"               [-r [regfile]] [-s [statusfile]]\n"); 
    fprintf(stderr,"               [-p progfile]\n");
    fprintf(stderr,"        if optional filenames are not given, data is\n"); 
    fprintf(stderr,"        written to standard output\n");
    fprintf(stderr,"        -k [keyfile]         : Write user keys\n");
    fprintf(stderr,"        -b buff# [buff_file] : Write buffer # buff#\n");
    fprintf(stderr,"        -r [regfile]         : Write user registers\n");
    fprintf(stderr,"        -s [statusfile]      : Write status information\n");
    fprintf(stderr,"        -p progfile          : Write programs to files \n");
    fprintf(stderr,"                               progfile.001, progfile.002");
    fprintf(stderr,"...\n");
    exit(1);
  }

void get_filename(int argc, char **argv, int *option, outtype *out, 
                  char optchar) 
/* Process optional file names on the command line. If optchar is non-zero
   then the filename is not optional and a suitable error will be given if
   it is missing. Set 'out' to the appropriate values */
  {
    (*option)++;
    out->flag=1; /* set flag to indicate this area should be output */
    if(((*option)<argc) && (argv[*option][0]!='-'))
      {
        /* There is a filename, record it */
        strncpy(out->name, argv[*option], 80);
        (*option)++;
      }
    else
      {
        /* No filename */
        if(optchar)
          {
            /* but one is needed, give error */
            fprintf(stderr,"Filename required for the -%c\n\n",optchar);
            usage(); /* and quit */
          }
      }
  }

int main(int argc, char **argv)
  {
    /* Output data for main sections */
    outtype keys={"",0}, regs={"",0}, status={"",0}, prog={"",0};
    /* Output data for HP41 buffers */
    outtype buffers[16]={ {"",0},{"",0},{"",0},{"",0},{"",0},{"",0},{"",0},
      {"",0}, {"",0} ,{"",0},{"",0},{"",0},{"",0},{"",0},{"",0},{"",0}};
    int option; /* command line option */
    int buffno; /* buffer number given on command line */
    int bytes_read; /* File length in bytes */
    int curtain; /* register position of the curtain */
    int sreg; /* register position of 1st statistics register */
    int global_end; /* register postion of .END. */

#ifdef _WIN32
    setmode(fileno(stdin), O_BINARY);
#endif


    /* Process command line options */
    /* There must be at least one */
    if(argc<2)
      {
        usage();
      }
    option=1;
    while(option<argc)
      {
        /* Is this an option character ? */
        if((argv[option][0]!='-') || (strlen(argv[option])!=2))
          {
            /* No : give error and quit */
            fprintf(stderr,"Unrecognised option : %s\n\n",argv[option]);
            usage();
          }
        /* This is possibly a valid option */
        switch(argv[option][1])
          {
            case 'k' : /* Set flag for key definitions */
                       get_filename(argc,argv,&option,&keys,0);
                       break;
            case 'r' : /* Set flag for registers */
                       get_filename(argc,argv,&option,&regs,0);
                       break;
            case 's' : /* Set flag for status registers */
                       get_filename(argc,argv,&option,&status,0);
                       break;
            case 'p' : /* Set flag for programs */
                       get_filename(argc,argv,&option,&prog,'p');
                       break;
            case 'b' : /* Buffers : first check there's a buffer number */
                       option++;
                       if((option>=argc) || (argv[option][0]=='-'))
                         {
                           /* There isn't, give error and quit */
                           fprintf(stderr,"Missing buffer number\n\n");
                           usage();
                         }
                       /* Get the buffer number */
                       buffno=((int)strtol(argv[option],NULL,16))&0xf;
                       /* Set the flag */
                       get_filename(argc,argv,&option,&(buffers[buffno]),0);
                       break;
            default  : /* Unrecognised option */
                       fprintf(stderr,"Unrecogined option : %s\n\n",
                               argv[option]);
                       /* Fall thru */
            case '?' : usage();
          }
      }
    /* read in file */
    bytes_read=read_file();

    /* Check the length is reasonable */
    if((bytes_read<HP41C_SIZE) || (bytes_read>HP41CV_SIZE))
      {
        /* No, it isn't */
        fprintf(stderr,"This is not a Write-All file!\n");
        exit(1);
      }    

    /* Calculate important positions in memory */
    /* See the Syntetic Programming Quick Reference Guide for details
       of status register 13 (which is what is decoded here) */
    curtain=correct_for_gap((memory[7*13+4]<<4)+(memory[7*13+5]>>4));
    sreg=correct_for_gap((memory[7*13]<<4)+(memory[7*13+1]>>4));
    global_end=correct_for_gap(((memory[7*13+5]&0xf)<<8)+memory[7*13+6]);
    
    /* Now start decoding the actual information in the file */
    /* If specified, output a status file */
    if(status.flag)
      {
        output_status(status.name,(bytes_read/7)-curtain,sreg-curtain);
      }

    /* Output keys and buffers */
    output_keys_buffs(keys,buffers,global_end);

    /* If specified, output the user programs */
    if(prog.flag)
      {
        output_progs(prog.name,curtain,global_end);
      }

    /* If specified, output user data registers */
    if(regs.flag)
      {
        output_registers(regs.name,curtain,bytes_read);
      }

    exit(0);
  }
