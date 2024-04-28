/*
User-Code File Converter/Compiler/De-compiler/Bar-Code Generator.
Copyright (c) Leo Duran, 2000-2007.  All rights reserved.

Build environment: Microsoft Visual C++ 1.52c  16-bit compiler.
To build, run: nmake
*/

/*
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

typedef struct {
   char *prefix;
   int  code;
} FCN;

typedef struct {
   char *prefix;
   int  code[ 2 ];
} XROM;

// compiler states
typedef enum {
   COMPILE_SEEK_START_LINE,
   COMPILE_GET_LINE,
   COMPILE_SEEK_END_LINE,
   COMPILE_LINE_OUTPUT,
   COMPILE_IGNORE_BAD_LINE,
} COMPILE_STATE;

FCN alt_fcn1[] = {
   {"Σ+",      0x47},
   {"SIGMA+",  0x47},
   {"Σ-",      0x48},
   {"SIGMA-",  0x48},
   {"P->R",    0x4E},
   {"R->P",    0x4F},
   {"X**2",    0x51},
   {"Y**X",    0x53},
   {"E**X",    0x55},
   {"10**X",   0x57},
   {"E**X-1",  0x58},
   {"X!=0?",   0x63},
   {"X<>0?",   0x63},
   {"D->R",    0x6A},
   {"R->D",    0x6B},
   {"CLΣ",     0x70},
   {"CLSIGMA", 0x70},
   {"RUP",     0x74},
   {"X!=Y?",   0x79},
   {"X<>Y?",   0x79},
   {"ENTER^",  0x83},
};

FCN alt_fcn2[] = {
   {"STO+",     0x92},
   {"ST+",      0x92},
   {"STO-",     0x93},
   {"ST-",      0x93},
   {"STO*",     0x94},
   {"ST*",      0x94},
   {"STO/",     0x95},
   {"ST/",      0x95},
   {"ΣREG",     0x99},
   {"SIGREG",   0x99},
   {"SIGMAREG", 0x99},
};

XROM xrom_fcn[] = {
//
// -EXT FCN 2D
//
   {"ALENG",   { 25, 1 }},
};

char *single01_1C[] = {
             "LBL 00", "LBL 01", "LBL 02", "LBL 03", "LBL 04", "LBL 05", "LBL 06",
   "LBL 07", "LBL 08", "LBL 09", "LBL 10", "LBL 11", "LBL 12", "LBL 13", "LBL 14",
   "0",      "1",      "2",      "3",      "4",      "5",      "6",      "7",
   "8",      "9",      ".",      " E",     "-",
};

char *single20_8F[] = {
   "RCL 00", "RCL 01", "RCL 02", "RCL 03", "RCL 04", "RCL 05", "RCL 06", "RCL 07",
   "RCL 08", "RCL 09", "RCL 10", "RCL 11", "RCL 12", "RCL 13", "RCL 14", "RCL 15",
   "STO 00", "STO 01", "STO 02", "STO 03", "STO 04", "STO 05", "STO 06", "STO 07",
   "STO 08", "STO 09", "STO 10", "STO 11", "STO 12", "STO 13", "STO 14", "STO 15",
   "+",      "-",      "*",      "/",      "X<Y?",   "X>Y?",   "X<=Y?",  "S+",
   "S-",     "HMS+",   "HMS-",   "MOD",    "%",      "%CH",    "P-R",    "R-P",
   "LN",     "X^2",    "SQRT",   "Y^X",    "CHS",    "E^X",    "LOG",    "10^X",
   "E^X-1",  "SIN",    "COS",    "TAN",    "ASIN",   "ACOS",   "ATAN",   "DEC",
   "1/X",    "ABS",    "FACT",   "X#0?",   "X>0?",   "LN1+X",  "X<0?",   "X=0?",
   "INT",    "FRC",    "D-R",    "R-D",    "HMS",    "HR",     "RND",    "OCT",
   "CLS",    "X<>Y",   "PI",     "CLST",   "R^",     "RDN",    "LASTX",  "CLX",
   "X=Y?",   "X#Y?",   "SIGN",   "X<=0?",  "MEAN",   "SDEV",   "AVIEW",  "CLD",
   "DEG",    "RAD",    "GRAD",   "ENTER",  "STOP",   "RTN",    "BEEP",   "CLA",
   "ASHF",   "PSE",    "CLRG",   "AOFF",   "AON",    "OFF",    "PROMPT", "ADV",
};

char *prefix90_9F[] = {
   "RCL ",   "STO ",   "ST+ ",   "ST- ",   "ST* ",   "ST/ ",   "ISG ",   "DSE ",
   "VIEW ",  "SREG ",  "ASTO ",  "ARCL ",  "FIX ",   "SCI ",   "ENG ",   "TONE ",
};

char *prefixA8_AD[] = {
   "SF ",    "CF ",    "FS?C ",  "FC?C ",  "FS? ",   "FC? ",
};

char *prefixB1_BF[] = {
             "GTO 00", "GTO 01", "GTO 02", "GTO 03", "GTO 04", "GTO 05", "GTO 06",
   "GTO 07", "GTO 08", "GTO 09", "GTO 10", "GTO 11", "GTO 12", "GTO 13", "GTO 14",
};

char *prefixCE_CF[] = {

                                                               "X<> ",   "LBL ",
};

char prefixXROM[] = "XROM xx,xx";

char prefixGTO[] = "GTO ";

char prefixXEQ[] = "XEQ ";

char prefixGTO_IND[] = "GTO IND ";

char prefixXEQ_IND[] = "XEQ IND ";

char prefixLBL[] = "LBL ";

char prefixW[] = "W ";

char prefixEND[] = "END";

char prefixGTO_NOP[] = ";GTO __ (SPARE)";

char prefixGTO_IND_NOP[] = ";GTO IND __ (SPARE)";

char prefixXEQ_IND_NOP[] = ";XEQ IND __ (SPARE)";

char postfixIND[] = "IND ";

char *postfix00_7F[] = {
   "00",     "01",     "02",     "03",     "04",     "05",     "06",     "07",
   "08",     "09",     "10",     "11",     "12",     "13",     "14",     "15",
   "16",     "17",     "18",     "19",     "20",     "21",     "22",     "23",
   "24",     "25",     "26",     "27",     "28",     "29",     "30",     "31",
   "32",     "33",     "34",     "35",     "36",     "37",     "38",     "39",
   "40",     "41",     "42",     "43",     "44",     "45",     "46",     "47",
   "48",     "49",     "50",     "51",     "52",     "53",     "54",     "55",
   "56",     "57",     "58",     "59",     "60",     "61",     "62",     "63",
   "64",     "65",     "66",     "67",     "68",     "69",     "70",     "71",
   "72",     "73",     "74",     "75",     "76",     "77",     "78",     "79",
   "80",     "81",     "82",     "83",     "84",     "85",     "86",     "87",
   "88",     "89",     "90",     "91",     "92",     "93",     "94",     "95",
   "96",     "97",     "98",     "99",     "100",    "101",    "A",      "B",
   "C",      "D",      "E",      "F",      "G",      "H",      "I",      "J",
   "T",      "Z",      "Y",      "X",      "L",      "M",      "N",      "O",
   "P",      "Q",      "R",      "a",      "b",      "c",      "d",      "e",
};

char *alt_postfix102_111[] = {
                                                               "102",    "103",
   "104",    "105",    "106",    "107",    "108",    "109",    "110",    "111",
};

char *alt_postfix117_122[] = {
                                                     "[",      "\\",     "]",
   "^",      "_",      "`",
};

int get_line_args( char *line_argv[], char **line_ptr );

int compile_args( char *code_buffer,
                  int line_argc,
                  char *line_argv[] );

int compile_num( char *code, char *num );
int compile_text( char *code, char *text, int count );
int compile_alpha( char *code, char *prefix, char *alpha, int count );
int compile_arg1( char *code, char *prefix );
int compile_arg2( char *code, char *prefix, char *postfix );
int compile_arg3( char *code, char *prefix, char *pind, char *postfix );
int compile_label( char *code, char *label, char *alpha, int count, char *key );

int get_numeric_prefix( char *numeric, char *buffer );

int get_text_prefix( char *text, char *buffer, int *pcount );

int get_alpha_postfix( char *alpha, char *buffer, int *size );

int is_postfix( char *postfix, int *pindex );

int parse_text( char *text, char *buffer, int *pcount );

int is_inquotes( char *buffer );

int is_append( char *prefix );

int is_text( char *prefix );

int is_local_label( char *alpha );

int get_key( char *key );

char get_xdigit( char xdigit );

void init_xrom(void);

void read_xrom(char *name);

void compile_end( char *buffer, int bytes );
