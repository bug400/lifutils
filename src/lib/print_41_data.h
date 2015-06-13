/* print_41_data.h -- routines to print the contents of HP41 (and SDATA) 
    files, etc */
/* 2000 A. R. Duell and placed under the GPL */

void print_numeric(unsigned char *data, int length_flag);
/* Print a record as a number, optionally with a 12 digit mantissa */

void print_char(unsigned char c);
 /* print a character, 'normally' if printable, otherwise as a \nnn
    octal escape sequence */
 
void print_string(unsigned char *str);
/* Print a 6-character string, skipping leading nulls */

void print_bldspec(unsigned char *str);
/* The HP41 printer command BLDSPEC packs a 7*7 character dot matrix 
   definition into a 6 byte string, with one bit in the otherwise unused
   nybble. */

void print_binary(unsigned char c, int length);
/* Print length bits of c in binary, MSB first */

void print_flags(unsigned char *str);
/* Print 44 bits corresponding to flags 0-43. It is known (Synthetic
   Quick Reference Guide) that flag 0 is the MSB of the flag register
   (Status Register d). Assume that it is also the MSB when the data
   is moved into the string */

void print_alpha(unsigned char *data, int bldspec_flag, int flags_flag);
/* Print an alpha record, optionally as bldspec data or flags */  

void print_hex(unsigned char *data);
/* Print a record in hex */

void print_record(unsigned char *record, int length, int bldspec_flag, 
                  int flags_flag);
/* print a signle 8-byte record from an SDATA (or similar) file */
