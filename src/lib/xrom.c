/* xrom.c -- Various utility functions for xrom handling  */
/* 2015 J. Siebold, and placed under the GPL */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"

#define DEBUG 0
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

static struct {
         char *name; /* Pointer to function name */
          int rom;   /* ROM ID */
          int fn;    /* Function ID */
   } xrom_ids [32*64] ;

static int num_xrom_ids; /* Number of entries */

/* Initialize xrom storage */

void init_xrom(void)
   {
      num_xrom_ids=0;
   }

/* Look up function name, return packed rom- and function id or -1 */
int get_xrom_by_name(char * alpha)
   {
      int j,mm,ff,i;

      j = num_xrom_ids;
      for( i = 0; i < j; ++ i ) {
          if( strcasecmp( alpha,xrom_ids[ i ].name ) == 0 ) {
              mm = xrom_ids[i].rom;
              ff = xrom_ids[i].fn;
              debug_print("xrom %s -> %d %d \n",alpha,mm,ff);
              return((mm<<8)| ff  );
          }
      }
      return(-1);
   }
/* Look up rom- and function id, return name or NULL */
char * get_xrom_by_id(int mm, int ff)
   {
      int j,i;

      j= num_xrom_ids;
      for( i = 0; i < j; ++ i ) {
          if(xrom_ids[i].rom== mm && xrom_ids[i].fn == ff) 
             return(xrom_ids[i].name);
      }
      return((char *) NULL);
   }
   
void read_xrom(char *name)
  {
    /* Read in an XROM names file. This consists of lines, each consisting
       of 2 decimal numbers (ROM# and function#) and a name string, separated
       by whitespace. The name must not contain any whitespace */

    FILE *xrom_file; /* XROM file name */
    char line[80]; /* Line from that file */
    int rom, fn; /* ROM and function numbers */
    char this_name[40]; /* name from this line */
    char filename [256];
    char *buf;

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
        if(sscanf(line,"%d %d %39s",&rom,&fn,this_name)==3)
          {
	    buf=malloc ((strlen(this_name)+1)*sizeof(char));
            if (buf == NULL) exit(0);
            strcpy(buf,this_name);
            xrom_ids[num_xrom_ids].name=buf;
            xrom_ids[num_xrom_ids].rom= rom;
            xrom_ids[num_xrom_ids].fn= fn;
            num_xrom_ids++;
          }
      }
    /* close the file */
    fclose(xrom_file);
  }

