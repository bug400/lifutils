/* scramble_41.c -- scramble an 7 byte HP41 register into an 8 byte file
   record */
/* 2000 A. R. Duell, and placed under the GPL */

/* This routine is essentially the inverse of descramble_41.c. It takes a
   7 byte 'register' and turns it into an 8 byte record as used in many
   HP41 files. The bytes in the record contain the following nybbles from
   the register :
   6H 6L | 5L 0H | 0 0 | 4L 5H | 3L 4H | 2L 3H | 1L 2H | 0L 1H */

void scramble(unsigned char *reg, unsigned char *record)
  {
    /* scramble a register into a record */
    record[0]=reg[6];
    record[1]=(reg[0]>>4)+((reg[5]&0xf)<<4);
    record[2]=0;
    record[3]=(reg[5]>>4)+((reg[4]&0xf)<<4);
    record[4]=(reg[4]>>4)+((reg[3]&0xf)<<4);
    record[5]=(reg[3]>>4)+((reg[2]&0xf)<<4);
    record[6]=(reg[2]>>4)+((reg[1]&0xf)<<4);
    record[7]=(reg[1]>>4)+((reg[0]&0xf)<<4);
  }

