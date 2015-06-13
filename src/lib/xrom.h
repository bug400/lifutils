/* xrom.h -- Various utility functions for xrom info handling */
/* 2015 J. Siebold, and placed under the GPL */

void init_xrom(void);
/* initialize xrom subsystem */

int get_xrom_by_name(char * alpha);
/* lookup rom and function id by function name */

char * get_xrom_by_id(int mm, int ff);
/* lookup function name by rom id and function id */
   
void read_xrom(char *name);
/* load xrom files */
