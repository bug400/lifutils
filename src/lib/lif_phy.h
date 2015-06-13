/* lif_phy.h -- header file for physical LIF disk functions (LINUX) */
/* 2000, 2015 A. R. Duell, J. Siebold and placed under the GPL */

int lif_open_phy_device(char * devicename,int flags);
/* open a physical device in the specified mode */

void lif_close_phy_device(int device_id);
/* close a physical device */

void lif_read_phy_block(int input_device, int block, unsigned char *data);
/* read the logical block number block from input device and store the
   sector in the buffer *data */

void lif_write_phy_block(int output_device, int block, unsigned char *data);
/* write the logical block number block to output device and get the
   sector from the buffer *data */

void lif_recalibrate_phy_device(int device);
/* recalibrare floppy, seek to sector 0 */

void lif_seek_phy_device(int device, int cylinder);
/* seek to a specific cylinder on device */

void lif_read_phy_device(int device, int cylinder, int head, int sector,
              unsigned char *data);
/* Read one sector from LIF disk to data[] array */

void lif_write_phy_device(int device, int cylinder, int head, int sector,
               unsigned char *data);
/* Read one sector from data[] array to LIF disk*/

