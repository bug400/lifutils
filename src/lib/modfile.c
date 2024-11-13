/*=======================================================================

Author: Warren Furlow (email: warren@furlow.org)

License: PUBLIC DOMAIN - May be freely copied and incorporated into any work

Description:  Contains routines for conversion between HP-41 ROM image file formats

Background:
Each HP-41 ROM page is 4096 words in size and each word is 10 bits.  There are
16 pages in the address space, but pages can be bank switched by up to 4 banks.
The pages 0-7 are for system use and pages that go there are hard coded to that
location.  Pages 8-F are for plug-in module usage through the four physical
ports.  Each port takes up two pages (Page8=Port1 Lower,Page9=Port1 Upper, etc.).
Notethat some plug-in modules and peripherals are hard coded to map into certain 
system pages.

Supported File Formats:
ROM - This format is used by the V41 Emulator Release 7 and prior (Warren Furlow).
      It is always 8192 bytes with the High 2 bits followed by the Low 8 bits.

BIN - This format is used by Emu41 (J-F Garnier) and HP41EPC (HrastProgrammer).
      Note: HP41EPC uses BIN format but names them .ROM files.
      All bits are packed into 5120 bytes, but several consecutive pages may
      occupy the same file, so the file size could be a multiple of 5120.
      4 machine words are packed into 5 bytes:
      Byte0=Word0[7-0]
      Byte1=Word1[5-0]<<2 | Word0[9-8]
      Byte2=Word2[3-0]<<4 | Word1[9-6]
      Byte3=Word3[1-0]<<6 | Word2[9-4]
      Byte4=Word3[9-2]

LST - This format is a text file dump of the disassembled machine code generated
      by the Zenrom MCED utility.  The first 4 digit hex number is the absolute
      address and the second 3 digit hex number is the machine word.  The mnemonic
      normally appears after that.  For the purposes of file conversion, only the 
      machine word is actually used and only the first page is read from the file
      with the provided routines.
      Example:
      8000 158 M=C    
      8001 398 C=ST   
      8002 056 C=0    XS     
      8003 284 CF     7      

MOD - MOD File format is a more advanced multi-page format for containing an entire module or
      all operating system pages.  See MODFile.h  This format is used by V41 Release 8.

Sample usage: convert a lst file to a bin file:
  word *ROM;
  ROM=read_lst_file("test.lst");
  if (ROM==NULL)
    return;
  write_bin_file("test.bin",ROM);
  free(ROM);

Convert a bin file to a rom file:
  ROM=read_bin_file("test.bin",0);
  if (ROM==NULL)
    return;
  write_rom_file("test.rom",ROM);
  free(ROM);

MODULE FILE LOADER - for emulators
The exact loading procedure will be dependent on the emulator's implementation.  V41 uses a 
three pass process to find empty pages and ensure that the ROM attributes are correctly followed.
Feel free to copy and adapt the algorithm in LoadMOD() to your software.

First Pass:  Validate variables, go through each page in the mod file.  If there any PageGroups,
count the number of pages in each group using the appropriate array (LowerGroup[8], UpperGroup[0], etc).
This count is stored as a negative number.  In the second pass this value will be replaced with 
a positive number which will represent the actual page to be 

Second Pass: Go through each page again and find free locations for any that are grouped.  
If a page is the first one encountered in a group, find a block of free space for the group.  
For instance, Odd/Even will require two contigous spaces.  If a page is the second or subsequent 
one encountered in a group, then we already have the block of space found and simply need to 
stick it in the right place.  For instance, the lower page has already been loaded, then the upper 
page goes right after it.

Third Pass: Find a free location for any non-grouped pages.  This includes system pages which have
their page number hardcoded.

=========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <limits.h>
#include "config.h"
#include "modfile.h"

/******************************/
word *read_rom_file(char *FullFileName)
  {
  FILE *File;
  long FileSize;
  size_t SizeRead;
  word *ROM;
  int i;

  File=fopen(FullFileName,"rb");
  if (File==NULL)
    {
    fprintf(stderr,"Error: File Open Failed: %s\n",FullFileName);
    return(NULL);
    }
  fseek(File,0,SEEK_END);
  FileSize=ftell(File);
  fseek(File,0,SEEK_SET);
  if (FileSize!=8192)
    {
    fclose(File);
    fprintf(stderr,"Error: File Size Invalid: %s\n",FullFileName);
    fprintf(stderr,"  ROM file size is 8192 bytes");
    return(NULL);
    }
  ROM=(word*)malloc(sizeof(word)*0x1000);
  if (ROM==NULL)
    {
    fclose(File);
    fprintf(stderr,"Error: Memory Allocation\n");
    return(NULL);
    }
  SizeRead=fread(ROM,1,8192,File);
  fclose(File);
  if (SizeRead!=8192)
    {
    fprintf(stderr,"Error: File Read Failed: %s\n",FullFileName);
    free(ROM);
    return(NULL);
    }
  for (i=0;i<0x1000;i++)
    ROM[i]=(ROM[i]<<8)|(ROM[i]>>8);
  return(ROM);
  }

/*******************************/
int write_rom_file(char *FullFileName,word *ROM)
  {
  FILE *File;
  size_t SizeWritten;
  word *ROM2;
  int i;

  if (ROM==NULL)
    return(0);
  File=fopen(FullFileName,"wb");
  if (File==NULL)
    {
    fprintf(stderr,"Error: File Open Failed: %s\n",FullFileName);
    return(0);
    }
  ROM2=(word*)malloc(sizeof(word)*0x1000);
  if (ROM2==NULL)
    {
    fclose(File);
    fprintf(stderr,"Error: Memory Allocation\n");
    return(0);
    }
  for (i=0;i<0x1000;i++)
    ROM2[i]=(ROM[i]<<8)|(ROM[i]>>8);
  SizeWritten=fwrite(ROM2,1,8192,File);
  fclose(File);
  free(ROM2);
  if (SizeWritten!=8192)
    {
    fprintf(stderr,"Error: File Write Failed: %s\n",FullFileName);
    return(0);
    }
  return(1);
  }

/*******************************/
word *read_bin_file(char *FullFileName,int Page)
  {
  FILE *File;
  long FileSize;
  size_t SizeRead;
  byte *BIN;
  word *ROM;

  File=fopen(FullFileName,"rb");
  if (File==NULL)
    {
    fprintf(stderr,"Error: File Open Failed: %s\n",FullFileName);
    return(NULL);
    }
  fseek(File,0,SEEK_END);
  FileSize=ftell(File);
  fseek(File,0,SEEK_SET);
  if (FileSize%5120)
    {
    fclose(File);
    fprintf(stderr,"Error: File Size Invalid: %s\n",FullFileName);
    return(NULL);
    }
  BIN=(byte*)malloc(FileSize);
  if (BIN==NULL)
    {
    fclose(File);
    fprintf(stderr,"Error: Memory Allocation\n");
    return(NULL);
    }
  SizeRead=fread(BIN,1,FileSize,File);
  fclose(File);
  if ((long)SizeRead!=FileSize)
    {
    fprintf(stderr,"Error: File Read Failed: %s\n",FullFileName);
    free(BIN);
    return(NULL);
    }

  ROM=(word*)malloc(sizeof(word)*0x1000);
  if (ROM==NULL)
    {
    fprintf(stderr,"Error: Memory Allocation\n");
    return(NULL);
    }
  unpack_image(ROM,BIN+Page*5120);
  free(BIN);
  return(ROM);
  }

/*******************************/
int write_bin_file(char *FullFileName,word *ROM)
  {
  FILE *File;
  size_t SizeWritten;
  byte *BIN;

  if (ROM==NULL)
    return(0);
  File=fopen(FullFileName,"wb");
  if (File==NULL)
    {
    fprintf(stderr,"Error: File Open Failed: %s\n",FullFileName);
    return(0);
    }

  BIN=(byte*)malloc(5120);
  if (BIN==NULL)
    {
    fprintf(stderr,"Error: Memory Allocation\n");
    return(0);
    }
  pack_image(ROM,BIN);
  SizeWritten=fwrite(BIN,1,5120,File);
  fclose(File);
  free(BIN);
  if (SizeWritten!=5120)
    {
    fprintf(stderr,"Error: File Write Failed: %s\n",FullFileName);
    return(0);
    }
  return(1);
  }

/*******************************/
word *read_lst_file(char *FullFileName)
  {
  FILE *File;
  word *ROM;
  char LST[PATH_MAX];
  int i;
  word addr,mword;

  File=fopen(FullFileName,"rt");
  if (File==NULL)
    {
    fprintf(stderr,"Error: File Open Failed: %s\n",FullFileName);
    return(NULL);
    }
  ROM=(word*)malloc(sizeof(word)*0x1000);
  if (ROM==NULL)
    {
    fprintf(stderr,"Error: Memory Allocation\n");
    return(NULL);
    }
  i=0;
  while (fgets(LST,sizeof(LST),File)&&i<0x1000)
    {
    if (sscanf(LST,"%hx %hx",&addr,&mword)==2)
      ROM[i++]=mword;
    }
  fclose(File);
  if (i!=0x1000)
    {
    fprintf(stderr,"Error: File Size Invalid: %s\n",FullFileName);
    free(ROM);
    return(NULL);
    }
  return(ROM);
  }

/*******************************/
int write_lst_file(char *FullFileName,word *ROM,int Page)
  {
  FILE *File;
  int i;

  if (ROM==NULL)
    return(0);
  File=fopen(FullFileName,"wt");
  if (File==NULL)
    {
    fprintf(stderr,"Error: File Open Failed: %s\n",FullFileName);
    return(0);
    }

  for (i=0;i<0x1000;i++)
    fprintf(File,"%04X %03X\n",i+Page*0x1000,ROM[i]&0x03FF);
  fclose(File);
  return(1);
  }

/******************************/
int compare_rom_files(char *FullFileName1,char *FullFileName2)
  {
  word *ROM1,*ROM2;
  int res;
  ROM1=read_rom_file(FullFileName1);
  if (ROM1==NULL)
    return(0);
  ROM2=read_rom_file(FullFileName2);
  if (ROM2==NULL)
    {
    free(ROM1);
    return(0);
    }
  res=(0==memcmp(ROM1,ROM2,8192));
  free(ROM1);
  free(ROM2);
  return(res);
  }

/******************************/
void unpack_image(
  word *ROM,
  byte *BIN)
  {
  int i;
  word *ptr=ROM;
  if ((ROM==NULL)||(BIN==NULL))
    return;
  for (i=0;i<5120;i+=5)
    {
    *ptr++=((BIN[i+1]&0x03)<<8) | BIN[i];
    *ptr++=((BIN[i+2]&0x0F)<<6) | ((BIN[i+1]&0xFC)>>2);
    *ptr++=((BIN[i+3]&0x3F)<<4) | ((BIN[i+2]&0xF0)>>4);
    *ptr++=(BIN[i+4]<<2) | ((BIN[i+3]&0xC0)>>6);
    }
  }

/******************************/
void pack_image(
  word *ROM,
  byte *BIN)
  {
  int i,j;
  if ((ROM==NULL)||(BIN==NULL))
    return;
  for (i=0,j=0;i<0x1000;i+=4)
    {
    BIN[j++]=ROM[i]&0x00FF;
    BIN[j++]=((ROM[i+1]&0x003F)<<2) | ((ROM[i]&0x0300)>>8);
    BIN[j++]=((ROM[i+2]&0x000F)<<4) | ((ROM[i+1]&0x03C0)>>6);
    BIN[j++]=((ROM[i+3]&0x0003)<<6) | ((ROM[i+2]&0x03F0)>>4);
    BIN[j++]=(ROM[i+3]&0x03FC)>>2;
    }
  }

/******************************/
/* Returns 0 for success, 1 for open fail, 2 for read fail, 3 for invalid file, 4 for allocation error */
/******************************/
int output_mod_info(
  FILE *OutFile,         /* output file or set to stdout */
  char *FullFileName,
  int Verbose,           /* generate all info except FAT */
  int DecodeFat,
  byte **OutputBuff)         /* decode fat if it exists */
  {
  FILE *MODFile;
  unsigned long FileSize;
  size_t SizeRead;
  byte *pBuff;
  ModuleFileHeader *pMFH;
  int i;
  word page_addr=0;

  if (DecodeFat)
    Verbose=1;

  /* open and read MOD file into a buffer */
  MODFile=fopen(FullFileName,"rb");
  if (MODFile==NULL)
    {
    if (Verbose)
      fprintf(OutFile,"Error: File open failed: %s\n",FullFileName);
    return(1);
    }
  fseek(MODFile,0,SEEK_END);
  FileSize=ftell(MODFile);
  fseek(MODFile,0,SEEK_SET);
  if ((FileSize-sizeof(ModuleFileHeader))%sizeof(ModuleFilePage))
    {
    fclose(MODFile);
    if (Verbose)
      fprintf(OutFile,"Error: File size invalid: %s\n",FullFileName);
    return(3);
    }
  pBuff=(byte*)malloc(FileSize);
  if (pBuff==NULL)
    {
    fclose(MODFile);
    if (Verbose)
      fprintf(OutFile,"Error: Memory allocation\n");
    return(4);
    }
  SizeRead=fread(pBuff,1,FileSize,MODFile);
  fclose(MODFile);
  if (SizeRead!=FileSize)
    {
    if (Verbose)
      fprintf(OutFile,"Error: File read failed: %s\n",FullFileName);
    free(pBuff);
    return(2);
    }

  /* check header */
  pMFH=(ModuleFileHeader*)pBuff;
  if (FileSize!=sizeof(ModuleFileHeader)+pMFH->NumPages*sizeof(ModuleFilePage))
    {
    if (Verbose)
      fprintf(OutFile,"Error: File size invalid: %s\n",FullFileName);
    free(pBuff);
    return(3);
    }
  if (0!=strcmp(pMFH->FileFormat,MOD_FORMAT))
    {
    if (Verbose)
      fprintf(OutFile,"Error: File type unknown: %s\n",FullFileName);
    free(pBuff);
    return(3);
    }
  if (pMFH->MemModules>4 || pMFH->XMemModules>3 || pMFH->Original>1 || pMFH->AppAutoUpdate>1 ||
    pMFH->Category>CATEGORY_MAX || pMFH->Hardware>HARDWARE_MAX)    /* out of range */
    {
    if (Verbose)
      fprintf(OutFile,"Error: llegal value(s) in header: %s\n",FullFileName);
    free(pBuff);
    return(3);
    }

  if (!Verbose)
    {
    fprintf(OutFile,"%-20s %-30s %-20s\n",FullFileName,pMFH->Title,pMFH->Author);
    if (OutputBuff)
      *OutputBuff=pBuff;
    else
      free(pBuff);
    return(0);
    }

  /* output header info */
  fprintf(OutFile,"FILE NAME: %s\n",FullFileName);
  fprintf(OutFile,"FILE FORMAT: %s\n",pMFH->FileFormat);
  fprintf(OutFile,"TITLE: %s\n",pMFH->Title);
  fprintf(OutFile,"VERSION: %s\n",pMFH->Version);
  fprintf(OutFile,"PART NUMBER: %s\n",pMFH->PartNumber);
  fprintf(OutFile,"AUTHOR: %s\n",pMFH->Author);
  fprintf(OutFile,"COPYRIGHT (c) %s\n",pMFH->Copyright);
  fprintf(OutFile,"LICENSE: %s\n",pMFH->License);
  fprintf(OutFile,"COMMENTS: %s\n",pMFH->Comments);
  switch (pMFH->Category)
    {
    case CATEGORY_UNDEF:
      fprintf(OutFile,"CATEGORY: Not categorized\n");
      break;
    case CATEGORY_OS:
      fprintf(OutFile,"CATEGORY: Base operating system\n");
      break;
    case CATEGORY_APP_PAC:
      fprintf(OutFile,"CATEGORY: HP Application PAC\n");
      break;
    case CATEGORY_HPIL_PERPH:
      fprintf(OutFile,"CATEGORY: HP-IL related Modules and devices\n");
      break;
    case CATEGORY_STD_PERPH:
      fprintf(OutFile,"CATEGORY: Standard peripherals\n");
      break;
    case CATEGORY_CUSTOM_PERPH:
      fprintf(OutFile,"CATEGORY: Custom peripherals\n");
      break;
    case CATEGORY_BETA:
      fprintf(OutFile,"CATEGORY: BETA releases not fully debugged and finished\n");
      break;
    case CATEGORY_EXPERIMENTAL:
      fprintf(OutFile,"CATEGORY: Test programs not meant for normal usage\n");
      break;
    }
  switch (pMFH->Hardware)
    {
    case HARDWARE_NONE:
      fprintf(OutFile,"HARDWARE: None\n");
      break;
    case HARDWARE_PRINTER:
      fprintf(OutFile,"HARDWARE: 82143A Printer\n");
      break;
    case HARDWARE_CARDREADER:
      fprintf(OutFile,"HARDWARE: 82104A Card Reader\n");
      break;
    case HARDWARE_TIMER:
      fprintf(OutFile,"HARDWARE: 82182A Time Module or HP-41CX built in timer\n");
      break;
    case HARDWARE_WAND:
      fprintf(OutFile,"HARDWARE: 82153A Barcode Wand\n");
      break;
    case HARDWARE_HPIL:
      fprintf(OutFile,"HARDWARE: 82160A HP-IL Module\n");
      break;
    case HARDWARE_INFRARED:
      fprintf(OutFile,"HARDWARE: 82242A Infrared Printer Module\n");
      break;
    case HARDWARE_HEPAX:
      fprintf(OutFile,"HARDWARE: HEPAX Module\n");
      break;
    case HARDWARE_WWRAMBOX:
      fprintf(OutFile,"HARDWARE: W&W RAMBOX Device\n");
      break;
    case HARDWARE_MLDL2000:
      fprintf(OutFile,"HARDWARE: MLDL2000 Device\n");
      break;
    case HARDWARE_CLONIX:
      fprintf(OutFile,"HARDWARE: CLONIX-41 Module\n");
      break;
    }
  fprintf(OutFile,"MEMORY MODULES: %d\n",pMFH->MemModules);
  fprintf(OutFile,"EXENDED MEMORY MODULES: %d\n",pMFH->XMemModules);
  fprintf(OutFile,"ORIGINAL: %s\n",pMFH->Original?"Yes - unaltered":"No - this file has been updated by a user application");
  fprintf(OutFile,"APPLICATION AUTO UPDATE: %s\n",pMFH->AppAutoUpdate?"Yes - update this file when saving other data (for MLDL/RAM)":"No - do not update this file");
  fprintf(OutFile,"NUMBER OF PAGES: %d\n",pMFH->NumPages);

  /* go through each page */
  for (i=0;i<pMFH->NumPages;i++)
    {
    ModuleFilePage *pMFP;
    word ROM[0x1000];
    char ID[10];
    pMFP=(ModuleFilePage*)(pBuff+sizeof(ModuleFileHeader)+sizeof(ModuleFilePage)*i);

    /* output page info */
    fprintf(OutFile,"\n");
    unpack_image(ROM,pMFP->Image);
    fprintf(OutFile,"ROM NAME: %s\n",pMFP->Name);
    get_rom_id(ROM,ID);
    if (0==strcmp(pMFP->ID,ID))
      fprintf(OutFile,"ROM ID: %s\n",pMFP->ID);
    else
      fprintf(OutFile,"ROM ID: \"%s\" (ACTUAL ID: \"%s\")\n",pMFP->ID,ID);
    if ((pMFP->Page>0x0f && pMFP->Page<POSITION_MIN) || pMFP->Page>POSITION_MAX || pMFP->PageGroup>8 ||
      pMFP->Bank==0 || pMFP->Bank>4 || pMFP->BankGroup>8 || pMFP->RAM>1 || pMFP->WriteProtect>1 || pMFP->FAT>1 ||  /* out of range values */
      (pMFP->PageGroup && pMFP->Page<=POSITION_ANY) ||    /* group pages cannot use non-grouped position codes */
      (!pMFP->PageGroup && pMFP->Page>POSITION_ANY))      /* non-grouped pages cannot use grouped position codes */
      fprintf(OutFile,"WARNING: Page info invalid: %s\n",FullFileName);
    if (pMFP->Page<=0x0f)
      {
      fprintf(OutFile,"PAGE: %X - must be in this location\n",pMFP->Page);
      page_addr=pMFP->Page*0x1000;
      }
    else
      {
      fprintf(OutFile,"PAGE: May be in more than one location\n");
      switch (pMFP->Page)
        {
        case POSITION_ANY:
          fprintf(OutFile,"POSITION: Any page 8-F\n");
          break;
        case POSITION_LOWER:
          fprintf(OutFile,"POSITION: In lower relative to upper page\n");
          break;
        case POSITION_UPPER:
          fprintf(OutFile,"POSITION: In upper page relative to lower page\n");
          break;
        case POSITION_ODD:
          fprintf(OutFile,"POSITION: Any odd page (9,B,D,F)\n");
          break;
        case POSITION_EVEN:
          fprintf(OutFile,"POSITION: Any even page (8,A,C,E)\n");
          break;
        case POSITION_ORDERED:
          fprintf(OutFile,"POSITION: Sequentially in MOD file order\n");
          break;
        }
       }
    if (pMFP->PageGroup==0)
      fprintf(OutFile,"PAGE GROUP: 0 - not grouped\n");
    else
      fprintf(OutFile,"PAGE GROUP: %d\n",pMFP->PageGroup);
    fprintf(OutFile,"BANK: %d\n",pMFP->Bank);
    if (pMFP->BankGroup==0)
      fprintf(OutFile,"BANK GROUP: 0 - not grouped\n");
    else
      fprintf(OutFile,"BANK GROUP: %d\n",pMFP->BankGroup);
    fprintf(OutFile,"RAM: %s\n",pMFP->RAM?"Yes":"No");
    fprintf(OutFile,"Write Protected: %s\n",pMFP->WriteProtect?"Yes":"No or Not Applicable");
    if (!pMFP->RAM && pMFP->WriteProtect)
      fprintf(OutFile,"WARNING: ROM pages should not have WriteProtect set\n");
    fprintf(OutFile,"FAT: %s\n",pMFP->FAT?"Yes":"No");

    /* output FAT */
    if (pMFP->FAT)
      {
      fprintf(OutFile,"XROM: %d\n",ROM[0]);
      fprintf(OutFile,"FCNS: %d\n",ROM[1]);
      if (!DecodeFat)
        fprintf(OutFile,"(FAT Not Decoded)\n");
      }
    if (pMFP->FAT && DecodeFat)
      {
      word entry_num=0;
      word addr,jmp_addr;

      fprintf(OutFile,"XROM  Addr Function    Type\n");
      addr=2;
      while (entry_num<=ROM[1] && !(ROM[addr]==0 && ROM[addr+1]==0))  /* while entry number is less then number of entries and fat terminator not found */
        {
        jmp_addr=((ROM[addr]&0x0ff)<<8) | (ROM[addr+1]&0x0ff);
        fprintf(OutFile,"%02d,%02d %04X ",ROM[0],entry_num,jmp_addr+page_addr);
        if (ROM[addr]&0x200)
          fprintf(OutFile,"            USER CODE");
        else if (jmp_addr<0x1000)                               /* 4K MCODE def */
          {
          int addr2=jmp_addr;
          char ch,punct;
          int end,prompt;
          do
            {
            addr2--;
            decode_fatchar(ROM[addr2],&ch,&punct,&end);
            fprintf(OutFile,"%c",ch);
            }
          while (!end && addr2>jmp_addr-11);
          while (addr2>jmp_addr-11)                             /* pad it out */
            {
            fprintf(OutFile," ");
            addr2--;
            }
          fprintf(OutFile," 4K MCODE");
          /* function type */
          if (ROM[jmp_addr]==0)
            {
            fprintf(OutFile," Nonprogrammable");
            if (ROM[jmp_addr+1]==0)
              fprintf(OutFile," Immediate");
            else
              fprintf(OutFile," NULLable");
            }
          else
            fprintf(OutFile," Programmable");
          /* prompt type -high two bits of first two chars */
          prompt=(ROM[jmp_addr-1]&0x300)>>8;
          if (prompt && !(ROM[jmp_addr-2]&0x0080))
            prompt|=(ROM[jmp_addr-2]&0x300)>>4;
          switch (prompt)
            {
            case 0:     /* no prompt */
              break;
            case 1:
              fprintf(OutFile," Prompt: Alpha (null input valid)");
              break;
            case 2:
              fprintf(OutFile," Prompt: 2 Digits, ST, INF, IND ST, +, -, * or /");
              break;
            case 3:
              fprintf(OutFile," Prompt: 2 Digits or non-null Alpha");
              break;
            case 11:
              fprintf(OutFile," Prompt: 3 Digits");
              break;
            case 12:
              fprintf(OutFile," Prompt: 2 Digits, ST, IND or IND ST");
              break;
            case 13:
              fprintf(OutFile," Prompt: 2 Digits, IND, IND ST or non-null Alpha");
              break;
            case 21:
              fprintf(OutFile," Prompt: non-null Alpha");
              break;
            case 22:
              fprintf(OutFile," Prompt: 2 Digits, IND or IND ST");
              break;
            case 23:
              fprintf(OutFile," Prompt: 2 digits or non-null Alpha");
              break;
            case 31:
              fprintf(OutFile," Prompt: 1 Digit, IND or IND ST");
              break;
            case 32:
              fprintf(OutFile," Prompt: 2 Digits, IND or IND ST");
              break;
            case 33:
              fprintf(OutFile," Prompt: 2 Digits, IND, IND ST, non-null Alpha . or ..");
              break;
            }
          }
        else
          fprintf(OutFile,"            8K MCODE (Not decoded)");
        fprintf(OutFile,"\n");
        entry_num++;
        addr+=2;
        }

      /* interrupt vectors */
      fprintf(OutFile,"INTERRUPT VECTORS:\n");
      fprintf(OutFile,"Pause loop:                      %03X\n",ROM[0x0ff4]);
      fprintf(OutFile,"Main running loop:               %03X\n",ROM[0x0ff5]);
      fprintf(OutFile,"Deep sleep wake up, no key down: %03X\n",ROM[0x0ff6]);
      fprintf(OutFile,"Off:                             %03X\n",ROM[0x0ff7]);
      fprintf(OutFile,"I/O service:                     %03X\n",ROM[0x0ff8]);
      fprintf(OutFile,"Deep sleep wake up:              %03X\n",ROM[0x0ff9]);
      fprintf(OutFile,"Cold start:                      %03X\n",ROM[0x0ffa]);
      }

    fprintf(OutFile,"CHECKSUM: %03X",ROM[0x0fff]);
    if (compute_checksum(ROM)==ROM[0x0fff])
      fprintf(OutFile," (CORRECT)\n");
    else
      fprintf(OutFile," (INCORRECT - COMPUTED CHECKSUM: %03X)\n",compute_checksum(ROM));
    }

  fprintf(OutFile,"\n");
  if (OutputBuff)
    *OutputBuff = pBuff;
  else
    free(pBuff);
  return(0);
  }

/******************************/
/* Returns 0 for success, 1 for open fail, 2 for read fail, 3 for invalid file, 4 for allocation error */
/******************************/
int extract_roms(
  char *FullFileName,
  int LstForNSIM)
  {
  FILE *MODFile;
  unsigned long FileSize;
  size_t SizeRead;
  byte *pBuff;
  ModuleFileHeader *pMFH;
  int i,j;

  /* open and read MOD file into a buffer */
  MODFile=fopen(FullFileName,"rb");
  if (MODFile==NULL)
    return(1);
  fseek(MODFile,0,SEEK_END);
  FileSize=ftell(MODFile);
  fseek(MODFile,0,SEEK_SET);
  if ((FileSize-sizeof(ModuleFileHeader))%sizeof(ModuleFilePage))
    {
    fclose(MODFile);
    return(3);
    }
  pBuff=(byte*)malloc(FileSize);
  if (pBuff==NULL)
    {
    fclose(MODFile);
    return(4);
    }
  SizeRead=fread(pBuff,1,FileSize,MODFile);
  fclose(MODFile);
  if (SizeRead!=FileSize)
    {
    free(pBuff);
    return(2);
    }

  /* check header */
  pMFH=(ModuleFileHeader*)pBuff;
  if (FileSize!=sizeof(ModuleFileHeader)+pMFH->NumPages*sizeof(ModuleFilePage))
    {
    free(pBuff);
    return(3);
    }
  if (0!=strcmp(pMFH->FileFormat,MOD_FORMAT))
    {
    free(pBuff);
    return(3);
    }
  if (pMFH->MemModules>4 || pMFH->XMemModules>3 || pMFH->Original>1 || pMFH->AppAutoUpdate>1 ||
    pMFH->Category>CATEGORY_MAX || pMFH->Hardware>HARDWARE_MAX)    /* out of range */
    {
    free(pBuff);
    return(3);
    }

  /* go through each page */
  for (i=0;i<pMFH->NumPages;i++)
    {
    char ROMFileName[PATH_MAX];
    ModuleFilePage *pMFP;
    word ROM[0x1000];
    pMFP=(ModuleFilePage*)(pBuff+sizeof(ModuleFileHeader)+sizeof(ModuleFilePage)*i);
    /* write the ROM file */
    unpack_image(ROM,pMFP->Image);
    strcat(strcpy( ROMFileName, pMFP->Name), ".rom");
    for(j=0; j<(int) strlen(ROMFileName)-4;j++) {
       if (strchr("[]{}.<>|,;:#'\" -~+*\\?=()/&%",ROMFileName[j]) != (char *) NULL)
          ROMFileName[j]='_';
    }
    fprintf(stderr,"extracting rom \"%s\" to %s\n",pMFP->Name,ROMFileName);
    write_rom_file(ROMFileName,ROM);
    if(LstForNSIM)
      {
      strcat(strcpy( ROMFileName, pMFP->Name), ".lst");
      write_lst_file(ROMFileName,ROM,i);
      }
    }
  free(pBuff);
  return(0);
  }

/*******************************/
/* Sum all values and when bit sum overflows into 11th bit add 1. */
/* Then take 2's complement.  Source: Zenrom pg. 101 */
/*******************************/
word compute_checksum(word *ROM)
  {
  word checksum=0;
  int i;
  if (ROM==NULL)
    return(0);
  for (i=0;i<0xfff;i++)
    {
    checksum+=ROM[i];
    if (checksum>0x3ff)
      checksum=(checksum&0x03ff)+1;
    }
  return((~checksum+1)&0x3ff);
  }

/*******************************/
/* gets the ROM ID at the end of the ROM.  This is valid for most ROMs except O/S which seems to only use 0x0ffe */
/* ROM ID is lcd coded and may have punct bits set but only uses chars 0-3f (no halfnut) */
/* the high two bits are apparently meaningless although some ROMs have them set */
/*******************************/
void get_rom_id(
  word *ROM,
  char *ID)           /* output: provide char[9] */
  {
  char *ptr=ID;
  char punct;
  int i;
  for (i=0x0ffe;i>=0x0ffb;i--)
    {
    decode_lcdchar((word)(ROM[i]&0x00ff),ptr++,&punct);
    if (punct)
      *(ptr++)=punct;
    }
  *ptr=0;
  }

/*********************/
const char LCDtoASCII[]=
  {
  '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\',']', '^', '_',
  ' ', '!', '\"','#', '$', '%', '&', '\'','(', ')', '*', '+', '{', '-', '}', '/',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '~', ';', '<', '=', '>', '?',
  '~', 'a', 'b', 'c', 'd', 'e', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~',
  '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~',
  '~', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
  'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '~', '~', '~', '~', '~',
  };

/*********************/
/* Decodes a 9 bit LCD char into ASCII values where possible */
/* LDC char table: Mcode for Beginners, Emery, pg. 108 */
/*********************/
void decode_lcdchar(
  word lcdchar,                     /* LCD char in (bits 8-0) */
  char *ch,                         /* ASCII char out */
  char *punct)                      /* punctuation char out if any or ==0 */
  {
  char val=(char)(lcdchar&0x3f);    /* take off bits 6-8 */
  if (lcdchar&0x0100)               /* bit 8 set - second 4 rows */
    *ch=LCDtoASCII[val+0x040];
  else                              /* first 4 rows */
    *ch=LCDtoASCII[(int)val];
  switch (lcdchar&0x00c0)           /* punct bits 6,7 */
    {
    case 0:
      *punct=0;
      break;
    case 0x0040:
      *punct='.';
      break;
    case 0x0080:
      *punct=':';
      break;
    case 0x00c0:
      *punct=',';
      break;
    }
  }  

/*********************/
/* Decodes a 10 bit FAT char into ASCII values where possible */
/*********************/
void decode_fatchar(
  word tyte,                        /* tyte in */
  char *ch,                         /* PC char out */
  char *punct,                      /* punctuation char out if any or ==0 */
  int *end)
  {
  word lcdchar=tyte&0x003f;         /* ignore two high bits - used for prompting - Zenrom pg 100. */
  if (tyte&0x40)                    /* bit 6 is special chars (bit 8 in the LCD table) */
    lcdchar|=0x0100;
  decode_lcdchar(lcdchar,ch,punct);
  *end=(tyte&0x0080)?1:0;           /* bit 7 indicates end of name */
  }
