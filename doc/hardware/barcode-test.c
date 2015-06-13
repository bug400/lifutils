#include<stdio.h>

int main(int argc, char **argv)
  {
    int i;
    putchar(0xf); /* 16 bytes in the line */
    for(i=0; i<16; i++)
      {
        putchar(i+((15-i)<<4));
      }
    exit(1);
  }

