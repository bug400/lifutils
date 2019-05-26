/* lif_create_entry.h -- create a LIF directory entry */
/* 2000 A. R. Duell, and placed under the GPL */

int get_filetype(char *type_string);
/* Get the filetype code corresponding to the give file type string */

void create_entry(unsigned char *entry, char *name, unsigned int filetype,
                  unsigned int start, unsigned int length, int odd_flag);
/* Create a LIF directory entry for the specified file */

void put_lif_int(unsigned char *data, int length, unsigned int value);
/* Put an integer into the next <length> bytes of <data>, MSB first */

void put_time(unsigned char *entry);
/* put  time stamp value in to a directory entry */

int bcd (int value);
/* convert a 2 digit number to bcd) */

