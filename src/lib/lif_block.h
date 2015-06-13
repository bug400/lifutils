/* lif_block.c -- generic i/o layer for lif disk or image file */
/*  2000, 2015 A. R. Duell, J. Siebold and placed under the GPL */
int lif_open(char * filename,int flags,int mode, int physical);
/* open a file or physical device */

void lif_close(int fileno);
/* close a file or physical device */

int lif_truncate(int fileno);
/* truncate a file to zero length (ingnored for physical devices) */

void lif_read_block(int input_device, int block, unsigned char *data);
/* Read a block from fileno input_device.  block is the 
   number to read, data points to a 256 byte buffer to receive it */

void lif_write_block(int output_device, int block, unsigned char *data);
/* write a file block */

void lif_write_dir_entry(int output_device, int dir_start, int entry, unsigned char * dir_entry);
/* write a directory entry */

