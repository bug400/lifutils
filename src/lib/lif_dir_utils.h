/* lif_dir_utils.h -- Various utility functions for processing a LIF 
                      directory */
/* 2000 A. R. Duell, and placed under the GPL */

unsigned int get_lif_int(unsigned char *data,int length);
/* Read the next <length> bytes (MSB first) from <data> and turn into 
   an integer */

unsigned char bcd_to_dec(unsigned char bcd);
/* Turn a BCD encoded byte into its decimal equivalent */

int file_length(unsigned char *entry, char *file_type);
/* Get file length and type from a directory entry */

int check_name(char *name, int len);
/* check file name */

int check_filename(char *name);

int check_labelname(char *name);

int compare_names(char *entry, char *cmp_name);

int pad_name(char *entry, char *cmp_name);

int pad_label(char *entry, char *cmp_name);

