/* lif_img_win.c -- read/write a block (specified by logical block number) 
   to/from a lif disk (WIN32)
   2017 J. Siebold and placed under the GPL */

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include "lif_const.h"
#include "lif_img.h"

#define DEBUG 0
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

static HANDLE img_file_handle= (HANDLE) NULL;
/*
 Open LIF image file (windows compatibility). Because we have only one open
 LIF image file at any time this procedures only allow to open one file.
*/

int lif_open_img_file(char *filename, int flags, int mode)
  {
     /* check if a file was already opened */
     if (img_file_handle != (HANDLE) NULL)
       {
          fprintf(stderr,"Tried to open more than one LIF image file\n");
          exit(1);
     }
     /* open files according to file open flags */
     if ((flags & 0x3) == O_RDONLY) 
       {
          img_file_handle= CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
       }
      if ((flags & 0x3) == O_RDWR)
       {
          img_file_handle= CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
       }
      if ((flags & 0x3) == O_WRONLY)
       {
          img_file_handle= CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
       }
      if (img_file_handle == INVALID_HANDLE_VALUE) 
       {
          exit_error("Error: open LIF image file failed");
       }
      /* fake a unix file handle */
      return(4);
  }
/* close a file */
void lif_close_img_file(int descriptor)
  {
     if (img_file_handle== (HANDLE) NULL)
       {
          fprintf(stderr,"Error: tried to close a non existing file handle");
          exit(1);
       }
      CloseHandle(img_file_handle);
      img_file_handle= (HANDLE) NULL;
  }

/* truncate a file */
void lif_truncate_img_file(int lif_file)
  {
    DWORD seek_ret;
    BOOL eof_ret;

     if (img_file_handle== (HANDLE) NULL) 
       {
          fprintf(stderr,"Error: tried truncating a non existing file handle");
          exit(1);
       }

    /* Go to block 0 in the file */
    seek_ret=SetFilePointer(img_file_handle, (LONG) 0,NULL,0); 
    if(seek_ret == INVALID_SET_FILE_POINTER) 
       {
          exit_error("Error: seek in LIF image file failed");
       }
    
     /* EOF */
     eof_ret= SetEndOfFile(img_file_handle);
     if(! eof_ret) 
       {
          exit_error("Error: trunate a LIF image file failed");
       }
  }

/* read a certain file sector */
void lif_read_img_block(int input_file, int block, unsigned char *data)
  {
    DWORD seek_ret;
    BOOL read_ret;
    DWORD NumberOfBytesRead;

    /* Read one block from an image file */
    if (img_file_handle== (HANDLE) NULL )
       {
          fprintf(stderr,"Error: tried reading from a non existing file handle");
          exit(1);
       }

    /* Go to the right block in the file */
    seek_ret=SetFilePointer(img_file_handle, (LONG) (SECTOR_SIZE*block),NULL,0); 
    if(seek_ret == INVALID_SET_FILE_POINTER) 
       {
          exit_error("Error: seek in LIF image file failed");
       }

    /* Read block */
    read_ret= ReadFile(img_file_handle, data, (DWORD) SECTOR_SIZE, &NumberOfBytesRead, NULL);
    if(! read_ret) 
       {
          exit_error("Error: read from LIF image file failed");
       }
    if (NumberOfBytesRead != SECTOR_SIZE)
       {
          exit_error("Error: read from LIF image file failed");
       }
  }


void lif_write_img_block(int output_file, int block, unsigned char *data)
  {
    DWORD seek_ret;
    BOOL write_ret;
    DWORD NumberOfBytesWritten;

    /* Read one block from an image file */
    if (img_file_handle== (HANDLE) NULL) 
       {
          fprintf(stderr,"Error: tried writing to a non existing file handle");
          exit(1);
       }

    /* Go to the right block in the file */
    seek_ret=SetFilePointer(img_file_handle, (LONG) (SECTOR_SIZE*block),NULL,0); 
    if(seek_ret == INVALID_SET_FILE_POINTER) 
       {
          exit_error("Error: write to LIF image file failed");
       }
    debug_print("write to block %d\n",block); 

    /* Write block */
    write_ret= WriteFile(img_file_handle, data, (DWORD) SECTOR_SIZE, &NumberOfBytesWritten, NULL);
    if(! write_ret) 
       {
          exit_error("Error: write to LIF image file failed");
       }
    if (NumberOfBytesWritten != SECTOR_SIZE)
       {
          exit_error("Error: write to LIF image file failed");
       }
  }
void exit_error(char * msg)
{
	wchar_t buf[256];
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 
              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 256, NULL);
	fprintf(stderr,"%s : %ls\n",msg,buf);
	exit(1);
}
