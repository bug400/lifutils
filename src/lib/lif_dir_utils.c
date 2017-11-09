/* lif_dir_utils.c -- Various utility functions for processing a LIF 
                      directory */
/* 2000 A. R. Duell, and placed under the GPL */

#include <stdio.h>
#include <string.h>
#include "lif_const.h"

unsigned int get_lif_int(unsigned char *data,int length)
  {
    /* Read the next <length> bytes (MSB first) from <data> and turn into an 
       integer */

    unsigned int value;
    int i;

    value=0;    
    for(i=0; i<length; i++)
      {
        value=(value<<8)+(*(data+i));
      }
    return(value);
  }

unsigned char bcd_to_dec(unsigned char bcd)
  {
    return( (((bcd & 0xf0)>>4)*10)+(bcd & 0x0f));
  }

int hp71_length(unsigned char *entry)
  {
    /* Several HP71 files have the length in nybbles stored in 
       bytes 28..30 of the directory entry. */
    int length;
    /* find length in nybbles */
    length=(*(entry+28)) + 
           ((*(entry+29))<<8) +
           ((*(entry+30))<<16);
    /* Convert to nybbles */
    length = (length+1)/2;
    return(length);
  }

int file_length(unsigned char *entry, char *file_type)
  {
    /* Figure out the file length (in bytes) as best we can
       for the file described in entry. If file_type is not NULL then
       return a string there giving the file type */

    unsigned int file_type_code;
    char *type_string;
    int length;
    char hex_type[8];
 
    /* Get the file type code from the directory */
    file_type_code=get_lif_int(entry+10,2);
    switch(file_type_code)
      {
        /* HP71 types, from the HPIL interface owner's manual */
        case 1 : /* Text file */
        case 0xE0D1 : /* Secure text file -- The HPIL user manual and 
                         IDS volume 1 give this as 0xE0D5. But the ROM
                         listing in IDS volume 3 and thus the machine 
                         iteslf gives 0xE0D1 */
          type_string="TEXT";
          length=get_lif_int(entry+16,4) * 256;
          break;
        case 0x00FF: /* disabled HP-71 LEX file */
          type_string="D-LEX";
          length=hp71_length(entry);
        case 0xE0D0 : /* SDATA file */
          type_string="SDATA";
          length=get_lif_int(entry+28,2) * 8;
          break;
        case 0xE0F0 : /* Data file */
        case 0xE0F1 : /* Secure Data file */
          type_string="DAT71";
          length=((*(entry+28)) + ((*(entry+29))<<8)) /* #records */
                *((*(entry+30)) + ((*(entry+31))<<8)); /* record length */
          break;
        case 0xE204 : /* BIN file */
        case 0xE205 : /* Secure BIN file */
        case 0xE206 : /* Private BIN file */
        case 0xE207 : /* Secure Private BIN file */
          type_string="BIN71";
          length=hp71_length(entry);
          break;
        case 0xE208 : /* Lex file */
        case 0xE209 : /* Secure Lex file */
        case 0xE20A : /* Private Lex file */
        case 0xE20B : /* Secure Private Lex file */
          type_string="LEX71";
          length=hp71_length(entry);
          break;
        case 0xE20C : /* Key file */
        case 0xE20D : /* Secure Key file */
          type_string="KEY71";
          length=hp71_length(entry);
          break;
        case 0xE214 : /* BASIC file */
        case 0xE215 : /* Secure BASIC file */
        case 0xE216 : /* Private BASIC file */
        case 0xE217 : /* Secure Private BASIC file */
          type_string="BAS71";
          length=hp71_length(entry);
          break;
        case 0xE218 : /* FRAM file */
        case 0xE219 : /* Secure FRAM file */
        case 0xE21A : /* Private FRAM file */
        case 0xE21B : /* Secure Private FRAM file */
          type_string="FTH71";
          length=get_lif_int(entry+16,4) * 256;
          break;
        case 0xE21C : /* ROM file */
          type_string="ROM71";
          length=hp71_length(entry);
          break;
        case 0xE222 : /* Graphics file */
          type_string="GRA71";
          length=hp71_length(entry);
          break;
        case 0xE224: /* Address file ?? */
          type_string="ADR71";
          length=hp71_length(entry);
          break;
        case 0xE22E: /* Symbol file ?? */
          type_string="SYM71";
          length=hp71_length(entry);
          break;

        /* HP41 types, from Synthetic Quick Reference Guide */
        /* It's not too clear which are in registers (and need * 8 in the 
           expression for length), and which are in bytes. This can be fixed
           later */
        case 0xE020 : /* WALL file with X-MEM */
          type_string="WAXM41";
          length=(get_lif_int(entry+28,2) * 8)+1;
        case 0xE030 : /* WALL file with X-MEM */
          type_string="XM41";
          length=(get_lif_int(entry+28,2) * 8)+1;
        case 0xE040 : /* WALL file */
          type_string="ALL41";
          length=(get_lif_int(entry+28,2) * 8)+1;
          break;
        case 0xE050 : /* KEYS file */
          type_string="KEY41";
          length=(get_lif_int(entry+28,2) * 8)+1;
          break;
        case 0xE060 : /* Status file */
          type_string="STAT41";
          length=(get_lif_int(entry+28,2) * 8)+1;
          break;
        case 0xE070 : /* HP41 ROM/MLDL dump file, as used by MLDL-OS */
          type_string="X-M41";
          length=(get_lif_int(entry+28,2) * 8)+1;
          break;
        case 0xE080 : /* Program file */
          type_string="PGM41";
          length=get_lif_int(entry+28,2)+1;
          break;
        /* HP75 types, from Synthetic Quick Reference Guide */
        /* Little is known about these, so expect bugs! */
        case 0xE052 : /* HP75 Text */
          type_string="TXT75";
          length=get_lif_int(entry+16,4) * 256;
          break;
        case 0xE053 : /* HP75 Appointments */
          type_string="APP75";
          length=get_lif_int(entry+16,4) * 256;
          break;
        case 0xE058 : /* HP75 Data */
          type_string="DAT75";
          length=get_lif_int(entry+16,4) * 256;
          break;
        case 0xE089 : /* HP75 Lex */
          type_string="LEX75";
          length=get_lif_int(entry+16,4) * 256;
          break;
        case 0xE08A : /* HP75 Visicalc */
          type_string="WKS75";
          length=get_lif_int(entry+16,4) * 256;
          break;
        case 0xE0FE : /* HP75 BASIC (HP41 SQRG says it is, but never seen) */
        case 0xE088 : /* HP75 BASIC */
          type_string="BAS75";
          length=get_lif_int(entry+16,4) * 256;
          break;
        case 0xE08B : /* HP75 ROM-related file ? */
          type_string="ROM75";
          length=get_lif_int(entry+16,4) * 256;
          break;
        /* Unknown file type */
        default : 
          sprintf(hex_type,"?(%4X)",file_type_code);
          type_string=hex_type;
          length=get_lif_int(entry+16,4) * 256;
          break;
      }
    /* If there's a place to put it, copy the file type string */
    if(file_type) 
      {
        strcpy(file_type,type_string);
      }
    return(length);
  }

int compare_names(char *entry, char *cmp_name)
  {
    /* Check if the file name in directory entry is the same as the one
       given in cmp_name. Return true if so, false otherwise */

    int i, equal;

    equal=1; /* Initially assume this is the right entry, until proved 
                otherwise */
    for(i=0; i<NAME_LEN; i++)
      {
        if(entry[i]!=cmp_name[i])
          {
            equal=0; /* If the characters don't match, this is not the right
                        entry */
          }
      }
    return(equal);
  }


void pad_name(char *name, char *cmp_name)
  {
    /* Pad a filename with spaces, so as to be able to compare it with
       a LIF directory record. On entry, cmp_name points to a 10 character
       string. On exit, the first characters of this contain the 
       characters from name, the rest are spaces */

    int i; /* general index */
    for(i=0; i<NAME_LEN; i++)
      {
        cmp_name[i]=' '; /* fill cmp_name with spaces */
      }
    for(i=0; i<NAME_LEN; i++)
      {
        /* Quit at end of name */
        if(!name[i])
          {
            break;
          }
        /* Otherwise copy characters from name to cmp_name */
        cmp_name[i]=name[i];
      }
  }

void pad_label(char *name, char *cmp_name)
 {
    /* Pad a label name with spaces
       On entry, cmp_name points to a 6 character
       string. On exit, the first characters of this contain the 
       characters from name, the rest are spaces */

    int i; /* general index */
    for(i=0; i<LABEL_LEN; i++)
      {
        cmp_name[i]=' '; /* fill cmp_name with spaces */
      }
    for(i=0; i<LABEL_LEN; i++)
      {
        /* Quit at end of name */
        if(!name[i])
          {
            break;
          }
        /* Otherwise copy characters from name to cmp_name */
        cmp_name[i]=name[i];
      }
  }

int check_name(char *name, int len)
  {
      int i;

      if(strlen(name) == 0 || strlen(name) > len) return (0);
      if(name [0] < 'A' || name [0] > 'Z') return (0);
      for (i=1; i< strlen(name); i++) {
         if(name[i] < 'A' || name[i] > 'Z')  
            if (name[i] < '0' || name[i] > '9') return(0);
      }
      return(1);
  }

int check_filename(char *name)
  {
      return check_name(name,NAME_LEN);
  }

int check_labelname(char *name)
  {
      return check_name(name,LABEL_LEN);
  }

