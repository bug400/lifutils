/* main LIFUTILS include file with various definitions used in several utilities 
   2024 J. Siebold and placed under the GPL */

#ifndef LIFUTILS_H
#define LIFUTILS_H

#define TRUE 1
#define FALSE 0

/* exit() parameters */
#define EXIT_OK  0            
#define EXIT_ERROR 1

/* return status parameters */
#define RETURN_OK 0           // all values >=0 are OK
#define RETURN_ERROR -1

#define BARCODE_LENGTH  16    // max. length of a barcode row in bytes
#define SDATA_RECORD_SIZE 8   // record size of HP41 sdata file
#define REGISTER_SIZE 7       // length of HP41 register

/* Smallest and largest byte counts of HP41 memory */
#define HP41C_SIZE 560
#define HP41CV_SIZE 2352

#define   PAGE_SIZE 4096*4
#define   LIF_ROM41_SIZE ((PAGE_SIZE / 4) * 5)
#define   ROM_RECORD_SIZE 4
#define   SCRAMBLED_ROM_RECORD_SIZE 5

#define MEMORY_SIZE  4096     // is larger than any real HP41's memory, so any valid program will fit 

/* offsets for the parts of a timestamp */
#define YEAR_OFF 0
#define MONTH_OFF 1
#define DAY_OFF 2
#define HOUR_OFF 3
#define MINUTE_OFF 4
#define SECOND_OFF 5


#define filelength_in_blocks(i) \
   i/SECTOR_SIZE + (int) ((i % SECTOR_SIZE) != 0)

#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

#define return_if_error(x) \
            if((x) == RETURN_ERROR) return(RETURN_ERROR)

int barps(int argc, char **argv);
int barprt(int argc, char **argv);
int comp41(int argc, char **argv);
int decomp41(int argc, char **argv);
int er41rom(int argc, char **argv);
int hx41rom(int argc, char **argv);
int in71(int argc, char **argv);
int inp41(int argc, char **argv);
int key41(int argc, char **argv);
int lexcat71(int argc, char **argv);
int lexcat75(int argc, char **argv);
int lifdir(int argc, char **argv);
int liffix(int argc, char **argv);
int lifget(int argc, char **argv);
int lifinit(int argc, char **argv);
int liflabel(int argc, char **argv);
int lifpack(int argc, char **argv);
int lifpurge(int argc, char **argv);
int lifput(int argc, char **argv);
int lifrename(int argc, char **argv);
int lifstat(int argc, char **argv);
int lifheader(int argc, char **argv);
int lifmod(int argc, char **argv);
int lifraw(int argc, char **argv);
int liftext(int argc, char **argv);
int liftext75(int argc, char **argv);
int outp41(int argc, char **argv);
int out71(int argc, char **argv);
int prog41bar(int argc, char **argv);
int raw41lif(int argc, char **argv);
int regs41(int argc, char **argv);
int rom41cat(int argc, char **argv);
int rom41er(int argc, char **argv);
int rom41lif(int argc, char **argv);
int rom41hx(int argc, char **argv);
int sdata(int argc, char **argv);
int sdatabar(int argc, char **argv);
int stat41(int argc, char **argv);
int textlif(int argc, char **argv);
int textlif75(int argc, char **argv);
int wall41(int argc, char **argv);
int wcat41(int argc, char **argv);

void add_to_checksum(int *checksum, unsigned char data);
int read_prog(FILE *fp,unsigned char *memory);
void print_char_hex(unsigned char c);
void print_byte_hex(unsigned char byte);
void print_date(unsigned char *date);
int correct_for_gap(int value);
#endif

