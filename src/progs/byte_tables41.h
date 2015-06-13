/* byte_tables41.h -- Instruction name tables for the non-regular HP41
   instructions */
/* 2000 A. R. Duell, and placed under the GPL */

/* Single Byte functions from 0x40--0x8f */
char *single_byte[80]={
"+", "-", "*", "/", "X<Y?", "X>Y?", "X<=Y?", "S+",
"S-", "HMS+", "HMS-", "MOD", "%", "%CH", "P-R", "R-P",
"LN", "X^2", "SQRT", "Y^X", "CHS", "E^X", "LOG", "10^X",
"E^X-1", "SIN", "COS", "TAN", "ASIN", "ACOS", "ATAN", "DEC",
"1/X", "ABS", "FACT", "X#0?", "X>0?", "LN1+X", "X<0?", "X=0?",
"INT", "FRC", "D-R", "R-D", "HMS", "HR", "RND", "OCT",
"CLS", "X<>Y", "PI", "CLST", "R^", "RDN", "LASTX", "CLX",
"X=Y?", "X#Y?", "SIGN","X<=0?", "MEAN", "SDEV", "AVIEW", "CLD",
"DEG", "RAD", "GRAD", "ENTER^", "STOP", "RTN", "BEEP", "CLA",
"ASHF", "PSE", "CLRG", "AOFF", "AON", "OFF", "PROMPT", "ADV"};

/* Double Byte functions from 0x90-0xaf */
char *double_byte[32]={
"RCL", "STO", "ST+", "ST-", "ST*", "ST/", "ISG", "DSE",
"VIEW", "SREG", "ASTO", "ARCL", "FIX", "SCI", "ENG", "TONE",
"XR", "XR", "XR", "XR", "XR", "XR", "XR", "XR",
"SF", "CF", "FS?C", "FC?C", "FS?",  "FC?", "GX", "??"};

/* Alpha goto/exec from 0x1d to 0x1f */
char *alpha_gto[3]={
"GTO", "XEQ", "W"};

/* Swap and local labels from 0xce to 0xcf */
char *row_c[2]={
"X<>", "LBL"};

/* Alpha suffixes from 0x66 to 0x7f */
char suffix[26]={
'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
'T', 'Z', 'Y', 'X', 'L', 'M', 'N', 'O', 'P', 'Q',
'R', 'a', 'b', 'c', 'd', 'e'};

/* Digit entry functions from 0x10 to 0x1c */
char digit[13]={
'0','1','2','3','4','5','6','7','8','9','.','E','-'};

