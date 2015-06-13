/* lif_img.h -- functions to read/write one block from/to a lif image file  */
/* 2000, 2015 A. R. Duell, J. Siebold and placed under the GPL */

int lif_open_img_file(char * filename, int mode, int flag);
/* open an image file.  filename is the name of the file or device,
   mode is the mode in which it is opened and flag is 1 if filename
   is a device and 0 if filename is a file */

void lif_close_img_file(int fileno);
/* Close the file or device indicated by descriptor */

void lif_read_img_block(int input_file, int block, unsigned char *data);
/* Read a block from descriptor input_device.  block is the 
   number to read, data points to a 256 byte buffer to receive it */

void lif_write_img_block(int output_file, int block, unsigned char *data);
/* write a file block to descriptor output_device.  block is the 
   number to write, data points to a 256 byte buffer  */

void lif_truncate_img_file(int fileno);
/* truncate an image file to zero length */
