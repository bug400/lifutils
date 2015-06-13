/* descramble_41.c -- descramble an 8-byte HP41 file record into a
   7 byte 'register' */
/* 2000 A. R. Duell, and placed under the GPL */

/* Several HP41 file types consist of 8 byte records which each represent 
   a 7 byte HP41 register. The register consists of the following nybbles 
   from the file record : 
   1L 7H | 7L 6H | 6L 5H | 5L 4H | 4L 3H | 3L 1H | 0H 0L */

void descramble(unsigned char *record, unsigned char *reg)
  {
    /* Descramble a record into a register */
    reg[0]=((record[1]&0xf)<<4)+(record[7]>>4);
    reg[1]=((record[7]&0xf)<<4)+(record[6]>>4);
    reg[2]=((record[6]&0xf)<<4)+(record[5]>>4);
    reg[3]=((record[5]&0xf)<<4)+(record[4]>>4);
    reg[4]=((record[4]&0xf)<<4)+(record[3]>>4);
    reg[5]=((record[3]&0xf)<<4)+(record[1]>>4);
    reg[6]=record[0];
  }

