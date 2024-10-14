/* xrom.c -- Various utility functions for xrom handling  */
/* 2015, 2024 J. Siebold, and placed under the GPL */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "config.h"
#include "xrom.h"

static char hpstring[MAX_ALPHA*5]; /* max string length if all chars are hex numbers */

#define DEBUG 0
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

static struct {
          char *name;     /* Pointer to function name */
          char *alt_name; /* Pointer to alternate function name, points to zero length string if alternate 
                             name does not exists */
          int rom;        /* ROM ID */
          int fn;         /* Function ID */
   } xrom_ids [32*64] ;

static int num_xrom_ids; /* Number of entries */

#define MAX_XROMNAME 21

/* check, whether HP 41 text bytes contain characters that can be converted to UTF */

int has_special_characters(unsigned char *str, int len)
{
   for(int i=0; i< len; i++) {
      if(str[i]== 0x0c || str[i] == 0x0d || str[i] == 0x1d || str[i] == 0x7e) return(1);
   }
   return(0);
}

/* convert HP-41 text bytes to a printable string. Some characters are are converted to UTF if utf flag
   is set. Non printable characters are output as escaped hex numbers. */

char * to_hp41_string(unsigned char *str, int len, int utf)
{
   int i,j,k;
   unsigned char c;

   j=0;
   if(len> MAX_ALPHA) return ((char *) NULL);
   for(i=0;i<len;i++) {
      c=str[i];
      // convert special character to UTF (append not handled here)
      if(c==0x0c && utf) {
         k=sprintf(hpstring+j,"%s","μ");
         j+=k;
      } else if(c==0x0d && utf) {
//       k=sprintf(hpstring+j,"%s","∡");
         k=sprintf(hpstring+j,"%s","⊀");  // because rpncomp uses that too
         j+=k;
      } else if(c==0x1d && utf) {
         k=sprintf(hpstring+j,"%s","≠");
         j+=k;
      } else if(c==0x7e && utf) {
         k=sprintf(hpstring+j,"%s","Σ");
         j+=k;
      // printable character, but escape backslash, ' and "
      } else if (c >= 0x20 && c <= 0x65) {
         if(c == '\\' || c == '\'' || c == '"') {
            hpstring[j++]='\\';
         }
         hpstring[j++]=c;
      // non printable character, print hex
      } else {
         k=sprintf(hpstring+j,"\\x%02x",c);
         j+=k;
      }
   }
   hpstring[j]='\0';
   return(hpstring);
}

/* compare strings which may have UTF-8 characters case insensitive */

int utfstrcasecmp(char * str1, char * str2) {

   int i;
   char c1,c2;

   if(strlen(str1) != strlen(str2)) return 1;

   for (i=0;i<strlen(str1);i++) {
      c1= str1[i];
      c2= str2[i];
      /* UTF byte, compare byte value only */
      if((c1 & 0xC0) == 0xC0) {
         if(c1 != c2) return 1;
      /* ASCII char, compare lowercase characters */
      } else {
         if(tolower(c1) != tolower(c2)) return 1;
      }
   }
   return 0;
}

/* Initialize xrom storage */

void init_xrom(void)
   {
      num_xrom_ids=0;
   }

/* Look up function name, return packed rom- and function id or -1 */
int get_xrom_by_name(char * alpha)
   {
      int j,mm,ff,i;

      debug_print("xrom lookup %s\n",alpha);
      j = num_xrom_ids;
      for( i = 0; i < j; ++ i ) {
          if((utfstrcasecmp( alpha,xrom_ids[ i ].name )== 0) || (utfstrcasecmp( alpha,xrom_ids[ i ].alt_name ) == 0)) {
              mm = xrom_ids[i].rom;
              ff = xrom_ids[i].fn;
              debug_print("xrom %s -> %d %d \n",alpha,mm,ff);
              return((mm<<8)| ff  );
          }
      }
      return(-1);
   }

/* Look up rom- and function id, return index in xrom_ids table or -1 */
int find_xrom_by_id(int mm, int ff)
   {
      int j,i;

      j= num_xrom_ids;
      for( i = 0; i < j; ++ i ) {
          if(xrom_ids[i].rom== mm && xrom_ids[i].fn == ff) 
             return(i);
      }
      return(-1);
   }

/* get xrom function name by table index */
char * get_xrom_name_by_index(int i)
   {
       return(xrom_ids[i].name);
   }

/* get xrom alternate function name by table index, return default function name if alternate name does not
   exist */
char * get_xrom_alt_name_by_index(int i)
   {
       if(strlen(xrom_ids[i].alt_name)> 0) {
          return(xrom_ids[i].alt_name);
       } else {
          return(xrom_ids[i].name);
       }
   }

void read_xrom(char *name)
  {
    /* Read in an XROM names file. This consists of lines, each consisting
       of 2 decimal numbers (ROM# and function#) and a name string, separated
       by whitespace. The name must not contain any whitespace */

    FILE *xrom_file; /* XROM file name */
    char line[100]; /* Line from that file */
    int rom, fn; /* ROM and function numbers */
    char this_name[40]; /* name from this line */
    char alt_name[40]; /* name from this line */
    char filename [256];
    char *buf, *alt_buf;
    int len;

    /* open the file */
    char *path;
    path=getenv("LIFUTILSXROMDIR");
#ifdef _WIN32
    if(path != (char *) NULL) {
       strcpy(filename,path);
       if(path[strlen(path)-1] != '\\') strcat(filename,"\\");
    } else {
      strcpy(filename,"");
    }
#endif
#ifdef __linux__
    if(path != (char *) NULL) {
       strcpy(filename,path);
       if(path[strlen(path)-1] != '/') strcat(filename,"/");
    } else {
       strcpy(filename,"/usr/share/lifutils/xroms/");
    }
#endif
#ifdef __APPLE__
    if(path != (char *) NULL) {
       strcpy(filename,path);
       if(path[strlen(path)-1] != '/') strcat(filename,"/");
    } else {
       strcpy(filename,"/usr/local/share/lifutils/xroms/");
    }
#endif

    strcat(filename,name);
    strcat(filename,".xrom");
    xrom_file=fopen(filename,"r");
    if(xrom_file == (FILE*) NULL)  {
        xrom_file=fopen(name,"r");
    }
    if(xrom_file == (FILE*) NULL)  return;

    while(fgets(line,80,xrom_file))
      {
        len=sscanf(line,"%d %d %s %s",&rom,&fn,this_name, alt_name);
        if(len==3 || len == 4) 
          {
	    buf=malloc ((strlen(this_name)+1)*sizeof(char));
            if (buf == NULL) exit(0);
            strcpy(buf,this_name);
            xrom_ids[num_xrom_ids].name=buf;
            xrom_ids[num_xrom_ids].rom= rom;
            xrom_ids[num_xrom_ids].fn= fn;
            if(len==4) {
               alt_buf= malloc ((strlen(alt_name)+1)*sizeof(char));
               strcpy(alt_buf,alt_name);
            } else {
               alt_buf= malloc (sizeof(char));
               strcpy(alt_buf,"");
            }
            xrom_ids[num_xrom_ids].alt_name=alt_buf;
            debug_print("%d %d %d %s %s\n",num_xrom_ids,xrom_ids[num_xrom_ids].rom, xrom_ids[num_xrom_ids].fn,
                  xrom_ids[num_xrom_ids].name,xrom_ids[num_xrom_ids].alt_name);
            num_xrom_ids++;
          }
      }
    /* close the file */
    fclose(xrom_file);
  }

