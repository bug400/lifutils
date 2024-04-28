/* xrom.h -- Various utility functions for xrom info handling */
/* 2015 J. Siebold, and placed under the GPL */

#define MAX_ALPHA 15

void init_xrom(void);
int get_xrom_by_name(char * alpha);
int find_xrom_by_id(int mm, int ff);
char * get_xrom_name_by_index(int ind);
char * get_xrom_alt_name_by_index(int ind);
void read_xrom(char *name);
char *to_hp41_string(unsigned char * str, int len, int utf);
