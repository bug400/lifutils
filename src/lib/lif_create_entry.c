/* lif_create_entry.c -- create a LIF directory entry */
/* 2000 A. R. Duell, and placed under the GPL */

#include<string.h>
#include<ctype.h>
#include<stdlib.h>
#include<time.h>

int get_filetype(char *type_string)
/* Get the filetype code corresponding to the give file type string */
  {
    struct ft 
      {
        char *name;
        int code;
      } filetype[] =
      { 
       {  "TEXT",   1 },
        { "SDATA",  0xE0D0 },
        { "DATA71", 0xE0F0 },
        { "BIN71",  0xE204 },
        { "LEX71",  0xE208 },
        { "KEY71",  0xE20C },
        { "BASIC71",0xE214 },
        { "FORTH71",0xE218 },
        { "WALL41", 0xE040 },
        { "KEY41",  0xE050 },
        { "STAT41", 0xE060 },
        { "ROM41",  0xE070 },
        { "PROG41", 0xE080 },
        { "TEXT75", 0xE052 },
        { "APPT75", 0xE053 },
        { "LEX75",  0xE089 },
        { "BASIC75",0xE088 },
        { "ROM75",0xE08B },
        { "",     0 }
       };
    char uc_ftype[11]; /* file type converted to upper case */
    int i; /* loop index */
    int type_code; /* File type code */

    /* convert input string to upper case */
    i=0;
    while((i<10) && (type_string[i]>' '))
      {
        uc_ftype[i]=toupper(type_string[i]);
        i++;
      }
    uc_ftype[i]=0;

    /* Search for matching filetype */
    i=0;
    type_code=-1; /* This will be made +ve when the file type is found */
    while(strlen(filetype[i].name))
      {
        if(!strcmp(uc_ftype,filetype[i].name))
          {
            type_code=filetype[i].code;
            break;
          }
        i++;
      }
    return(type_code);
  }

void put_lif_int(unsigned char *data, int length, unsigned int value)
/* Put an integer into the next <length> bytes of <data>, MSB first */
  {
    int i;
    for(i=length-1; i>=0; i--)
      {
        (*(data+i))=value&0xFF; /* Write next byte */
        value >>= 8;
      }
  }

void put_hp71_length(unsigned char *entry, unsigned int length, int odd_flag)
/* Many HP71 files have a 3 byte length (in nybbles) stored in bytes 
   28..30 of the directory entry */
  {
    length *=2; /* convert length to nybbles */
    /* If an odd number of nybbles, decrement length*/
    if(odd_flag) { length--;} 
    entry[28]=length&0xFF;
    length>>=8;
    entry[29]=length&0xFF;
    length>>=8;
    entry[30]=length&0xFF;
    entry[31]=0;
  }

int bcd(int value)
/* Convert a 2 digit number to BCD */
  {
    return((((value/10)%10)<<4)+(value%10));
  }

void put_time(unsigned char *entry)
/* Put the current real time and date into a directory entry */
  {
    time_t real_time;
    struct tm *decoded_time;

    /* this is for regression tests */
    if (getenv("LIFUTILSREGRESSIONTEST")!= (char *) NULL) {
        entry[20]=bcd(1);
        entry[21]=bcd(1);
        entry[22]=bcd(1);
        entry[23]=bcd(0);
        entry[24]=bcd(0);
        entry[25]=bcd(0);
        return;
    }

    real_time=time(0);
    decoded_time=localtime(&real_time);
    entry[20]=bcd((decoded_time->tm_year)%100);
    entry[21]=bcd(decoded_time->tm_mon+1);
    entry[22]=bcd(decoded_time->tm_mday);
    entry[23]=bcd(decoded_time->tm_hour);
    entry[24]=bcd(decoded_time->tm_min);
    entry[25]=bcd(decoded_time->tm_sec);
  }
     
void zero_time(unsigned char *entry)
/* Zero the time and data bytes in a directory entry */
  {
    int i;
    for(i=20; i<26; i++)
      {
        entry[i]=0;
      }
  }


void create_entry(unsigned char *entry, char *name, unsigned int filetype,
                  unsigned int start, unsigned int length, int odd_flag)
/* Create a LIF directory entry for the specified file */
  {
    int i; /* loop index */
    unsigned int blocks; /* number of blocks needed for this file */
    unsigned int imp_length; /* implementation-dependant length */

    /* Set up the file name */
    for(i=0; (i<10) && name[i]; i++)
      {
        entry[i]=name[i]; /* copy the file name over */
      }
    for( ; i<10; i++)
      {
        entry[i]=' '; /* and pad it with spaces */
      }

    /* Put the file type into the entry */
    put_lif_int(entry+10,2,filetype);
    /* Put the start block */
    put_lif_int(entry+12,4,start); 
    /* Put length in blocks */
    blocks=(length+255)/256; /* Always round up odd bytes */
    put_lif_int(entry+16,4,blocks);
    /* Fill in volume flag */
    entry[26]=128;
    entry[27]=1;
    /* File type dependant values */
    switch(filetype)
      {
        case 1: /* Text */
        case 0xE0D1: /* Secure text  -- is given as 0xE0D5 in the user
                         manual and IDS vol 1, but 0xE0D1 in IDC vol 3 */
          put_lif_int(entry+28,4,0);
          put_time(entry);
          break;
        case 0xE0D0: /* SDATA */
          imp_length=(length+7)/8;
          put_lif_int(entry+28,2,imp_length);
          put_lif_int(entry+30,2,0);
          put_time(entry);
          break;
        case 0xE0F0: /* DATA71 */
        case 0xE0F1: /* Secure DATA71 */
          entry[28]=length&0xFF; /* File length, assuming record length =1 */
          entry[29]=(length>>8)&0xFF;
          entry[30]=1; /* Record length */
          entry[31]=0;
          put_time(entry);
          break;
        case 0xE204: /* BIN71 */
        case 0xE205: /* Secure BIN71 */
        case 0xE206: /* Private BIN71 */
        case 0xE207: /* Secure Private BIN71 */
        case 0xE208: /* LEX71 */
        case 0xE209: /* Secure LEX71 */
        case 0xE20C: /* KEY71 */
        case 0xE20D: /* Secure KEY71 */
        case 0xE214: /* BASIC71 */
        case 0xE215: /* Secure BASIC71 */
        case 0xE216: /* Private BASIC71 */
        case 0xE217: /* Secure Private BASIC71 */
        case 0xE218: /* FORTH71 -- Should this be here? */
        case 0xE219: /* Secure FORTH71 */
        case 0xE21A: /* Private FORTH71 */
        case 0xE21B: /* Secure Private FORTH71 */
          put_hp71_length(entry,length,odd_flag);
          put_time(entry);
          break;
        case 0xE080: /* PROG41 */
          put_lif_int(entry+28,2,length);
          put_lif_int(entry+30,2,0);
          zero_time(entry);
          break;
        case 0xE050: /* KEY41 */
        case 0xE060: /* STAT41 */
        case 0xE070: /* ROM41 */
        case 0xE040: /* WALL41 */
          imp_length=(length+7)/8;
          put_lif_int(entry+28,2,imp_length);
          put_lif_int(entry+30,2,0);
          zero_time(entry);
          break;
        case 0xE052: /* TEXT75 */
        case 0xE053: /* APPT75 */
        case 0xE089: /* LEX75 */
        case 0xE088: /* BASIC75 */
        case 0xE08B: /* ROM75 */
          put_lif_int(entry+28,4,0); /* Clear the password field */
          put_time(entry);
          break;
       default : /* Unknown file type, clear type-depenedant bytes */ 
          put_lif_int(entry+28,4,0);
          put_time(entry);
          break;
      }
  }
