/* lifput.c -- Put a file to a LIF disk */
/* 2014 J.Siebold and placed under the GPL */

#include<stdio.h>
#include<fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "lifutils.h"
#include"lif_block.h"
#include"lif_dir_utils.h"
#include "lif_create_entry.h"
#include "lif_const.h"


#define DEBUG 0

#define filelength_in_blocks(i) \
   i/SECTOR_SIZE + (int) ((i % SECTOR_SIZE) != 0) 


void lifput_usage(void)
  {
    fprintf(stderr,"Usage : lifutils lifput [-l] [-p] LIFIMAGEFILE FILENAME\n");
    fprintf(stderr,"        or\n");
    fprintf(stderr,"        lifutils lifput [-l] [-p] LIFIMAGEFILE < input file\n");
    fprintf(stderr,"        Put a LIF file into a LIF image file\n");
    fprintf(stderr,"        -l Relax file name checking, allow underscores in file names.\n");
    fprintf(stderr,"        -p Write file to a floppy disk with a LIF file system.\n");
    fprintf(stderr,"           Note: this option is only supported on LINUX.\n");
    fprintf(stderr,"           Specify the floppy device instead of the lif-image-filename.\n");
    fprintf(stderr,"           See the LIFUTILS tutorial for details.\n");
    fprintf(stderr,"\n");
  }

/* read input file into buffer */

int lifput_read_input(FILE *fp, unsigned char **input_buffer,int num_blocks)
  {
    int i,l, bytes_read, padded_size;
    unsigned char record[SECTOR_SIZE];

    padded_size=num_blocks*SECTOR_SIZE;
    debug_print("padded size %d\n",padded_size);

    /* buffer in input file */
    *input_buffer= (unsigned char *) malloc(padded_size);
    if(*input_buffer== NULL) {
          fprintf(stderr,"cannot allocate buffer for input file\n");
          return(RETURN_ERROR);
    }
    bytes_read=0;
    while(1) {
       l=fread(record,sizeof(unsigned char),SECTOR_SIZE,fp);
       debug_print("bytes read %d l %d\n",bytes_read,l);
       if((bytes_read+l) > padded_size) {
           fprintf(stderr,"input file too large\n");
           free(*input_buffer);
           return(RETURN_ERROR);
       }
       if(l>0) {
           memcpy((*input_buffer)+bytes_read,record,l);
           bytes_read+=l;
       }
       if(l!= SECTOR_SIZE) break;
    }
    if(bytes_read < padded_size - SECTOR_SIZE) {
        fprintf(stderr,"premature end of input file\n");
        free(*input_buffer);
        return(RETURN_ERROR);
    }
    for(i=bytes_read;i<padded_size;i++) {
        *((*input_buffer)+i)=0;
    }
    return(bytes_read);
  }

void lifput_file_copy(int output_device, unsigned char * input_buffer, int start, int num_blocks)
  {
    /* Copy from buffer to output_file */
  
    int block,byte_counter;

    byte_counter=0;
    for(block=0; block<num_blocks; block++) {
       lif_write_block(output_device,start+block,input_buffer+byte_counter);
       byte_counter+= SECTOR_SIZE;
    }
  }

int lifput(int argc, char **argv)
  {
    /* System variables */
    int option; /* Command line option character */
    int output_device; /* Descriptor of output device */
    FILE *input_file; /* Input file stream */
    char cmp_name[NAME_LEN]; /* File name to look for */
    char chk_name[NAME_LEN+1]; /* File from input file to check */
    int data_length; /* length of input file in bytes without header */
    int num_blocks; /* size of input file in blocks */
    int physical_flag; /* Option to use a physical device */
    int lax; /* option to relax file name checking */
    struct blocktype {
       int startblock;
       int filelength;
    }  *blocklist, tempentry; /* list of blocks */
    int dir_entry_count; /* number of directory enries (including deleted) */
    int blocklist_count; /* number of entries in block list */
    int last_data_block; /* last data block in medium */
    int free_space; /* size of contiguous free blocks */
    int extend_dir;  /* flag if end of dir marker has to be moved */
    int fbytes;  /* file length of current directory entry */
    int fblocks; /* file length in blocks of current directory entry */
    int fstart;  /* start block of file of current directory entry */
    int bytes_read; /* number of bytes read from input */
    int i,j,n,t;

    
    /* LIF disk values */
    int dir_start; /* first block of the directory */
    int dir_length; /* length of directory in blocks */
    int medium_size; /* number of data blocks of medium */
    int tracks, surfaces, blocks;

    /* Directory search values */
    int dir_end; /* Set at end of directory */
    int found_file; /* Set when file found */
    int dir_entry; /* Directory entry within current block */
    int dir_block; /* Current block offset from start of directory */
    int file_type; /* file type word */
    unsigned char dir_data[SECTOR_SIZE]; /* Current directory block data */
    unsigned char * input_buffer; /* temporary buffer for input file without LIF header */

    /* new file entry values */
    int free_dir_entry; /* number of the new directory entry */
    int file_start; /* Start block number of the new file */
    unsigned char new_dir_entry[ENTRY_SIZE]; /* Directory Entry of new file */
    char typestring[10];

    /* Process command line options */
    physical_flag=0;
    lax=0;
    optind=1;
    while ((option=getopt(argc,argv,"pl?"))!=-1)
      {
        switch(option)
          {
            case 'p' : physical_flag=1;
                       break;

            case 'l' : lax=1;
                       break;

            case '?' : lifput_usage();
                       return(RETURN_OK);
          }
      }

    /* Are the right number of names specified ? */
    if( (optind != argc-1) && (optind != argc-2) )
      {
        /* No, give an error */
        lifput_usage();
        return(RETURN_ERROR);
      }

    /* Open output device */
    if((output_device=lif_open(argv[optind],O_RDWR| O_BINARY,0,physical_flag))==-1)
      {
        fprintf(stderr,"Error opening %s\n",argv[optind]);
        return(RETURN_ERROR);
      }

    /* Now read block 0 to find where the directory is */
    lif_read_block(output_device,0,dir_data);

    /* Make sure it's a LIF disk */
    if(get_lif_int(dir_data+0,2)!=0x8000)
      {
        fprintf(stderr,"This is not a LIF disk!\n");
        return(RETURN_ERROR);
      }

    /* Find the directory */
    dir_start=get_lif_int(dir_data+8,4);
    dir_length=get_lif_int(dir_data+16,4);

   /* get medium information */
   tracks=get_lif_int(dir_data+24,4);
   surfaces=get_lif_int(dir_data+28,4);
   blocks=get_lif_int(dir_data+32,4);
   medium_size= tracks*surfaces*blocks;
   if((tracks == surfaces) && (surfaces == blocks)) {
      fprintf(stderr,"Medium was not initialized properly\n");
      return(RETURN_ERROR);
    }

    debug_print("%s\n","medium Information");
    debug_print("medium size %d\n",medium_size);
    debug_print("dir_start %d dir_length %d\n\n",dir_start,dir_length);

    /* open input file, if none specified use standard input */
    if ( optind == argc -2) {
       input_file= fopen(argv[optind+1],"rb");
       if (input_file == (FILE *) NULL ) {
          fprintf(stderr,"can't open File %s\n",argv[optind+1]);
          return(RETURN_ERROR);
       }
    }
    else {
       SETMODE_STDIN_BINARY;
       input_file= stdin;
    }

    /* read lif entry from input file header */
    bytes_read=fread(new_dir_entry,sizeof(unsigned char),ENTRY_SIZE,input_file);
    if(bytes_read!=ENTRY_SIZE) {
          fprintf(stderr,"permature end of file\n");
          if(input_file!= stdin) fclose(input_file);
          return(RETURN_ERROR);
    }
    data_length= file_length(new_dir_entry,typestring);
    num_blocks= filelength_in_blocks(data_length);
    file_type= get_lif_int(&new_dir_entry[10],2);

    /* check file type */
    if(typestring[0]== '?') {
          fprintf(stderr,"illegal file type\n");
          if(input_file!= stdin) fclose(input_file);
          return(RETURN_ERROR);
    }

    bytes_read=lifput_read_input(input_file,&input_buffer,num_blocks);
    if(bytes_read== RETURN_ERROR) {
       if(input_file!=stdin) fclose(stdin);
          return(RETURN_ERROR);
    }
       

    /* set address to zero */
    new_dir_entry[12]=0;
    new_dir_entry[13]=0;
    new_dir_entry[14]=0;
    new_dir_entry[15]=0;

    /* get file name */
    for (i=0; i<NAME_LEN; i++) 
       cmp_name[i]= new_dir_entry[i];

    /* check the file name */
    for (i=0; i<NAME_LEN; i++) 
       if (new_dir_entry[i]==' ') 
          chk_name[i]= '\0';
       else
          chk_name[i]= new_dir_entry[i];
    chk_name[NAME_LEN]='\0';
    debug_print("File name to check: %s\n",chk_name);
    if (check_filename(chk_name,lax)==0) {
          fprintf(stderr,"illegal LIF file name\n");
          return(RETURN_ERROR);
    }
     
    debug_print("%s","input file: ");
    for (i=0; i<NAME_LEN; i++) debug_print("%c",cmp_name[i]);
    debug_print("%s","\n");

    debug_print("file type %x\n",file_type);
    debug_print("data_length %d\n",data_length);
    debug_print("num_blocks %d\n\n",num_blocks);
    

    /* Scan the directory, look for free diretory entries and
       build block list */
    dir_end=0;
    found_file=0;
    free_dir_entry=-1;
    dir_entry_count=0;
    blocklist_count=0;
    blocklist= malloc(8*dir_length*sizeof(struct blocktype));
    extend_dir= FALSE;
    debug_print("%s\n","directory scan");
    for(dir_block=0; dir_block<dir_length; dir_block++)
      {
        lif_read_block(output_device,dir_block+dir_start,dir_data);
        for(dir_entry=0; dir_entry<8; dir_entry++)
          {
            debug_print("dir entry no: %d\n",dir_entry_count);
            file_type=get_lif_int(dir_data+(dir_entry<<5)+10,2);

            /* Deleted file */
            if(file_type==0) {
               if(free_dir_entry== -1) free_dir_entry= dir_entry_count;
               debug_print("%s\n","deleted file");
            }
            else if(file_type==0xFFFF) {
               /* End of directory */
               debug_print("%s\n\n","end of dir mark");
               dir_end=1;
               if(free_dir_entry== -1) {
                  free_dir_entry= dir_entry_count;
                  /* do not move end of dir mark if we are in the last entry */
                  if(! (dir_entry_count == ((dir_length*8)-1)))
                     extend_dir= TRUE;
               }
               break;
            }
            else {
               fbytes=file_length(dir_data+(dir_entry<<5),NULL) ;
               fstart=get_lif_int(dir_data+(dir_entry<<5)+12,4);
               fblocks= filelength_in_blocks(fbytes);
               debug_print("Entry file type %04x startblock %d filelength %d (%d)\n",file_type, fstart,fbytes,fblocks);
               blocklist[blocklist_count].startblock= fstart;
               blocklist[blocklist_count].filelength= fblocks;
               blocklist_count++;

               if(compare_names((char *) dir_data+(dir_entry<<5),cmp_name)) {
                   /* Found the file */
                   found_file=1;
                   break;
                }
            }
            dir_entry_count+=1;
          }
        if(dir_end) { break; }; /* Quit at end */
      }

    if(found_file)
      {
        /* Give duplicate file error */
        fprintf(stderr,"Duplicate filename: ");
        for (i=0; i<NAME_LEN; i++) fprintf(stderr,"%c",cmp_name[i]);
        fprintf(stderr,"\n");
        free(blocklist);
        free(input_buffer);
        return(RETURN_ERROR);
      }

    /* insert pseudo file entry as end of file */
    blocklist[blocklist_count].startblock= medium_size+1;
    blocklist[blocklist_count].filelength=0;
    blocklist_count++;

    /* no free directory entry ? */
    if (free_dir_entry == -1) {
      fprintf(stderr,"Directory full\n");
      free(blocklist);
      free(input_buffer);
      return(RETURN_ERROR);
    }


    /* sort blocklist */
    j=1; t=1;
    n= blocklist_count;
    while (n-- && t) {
       for (j=t = 0; j < n; j++) {
          if (blocklist[j].startblock <= blocklist[j+1].startblock) continue;
          tempentry= blocklist [j];
          blocklist[j]= blocklist[j+1];
          blocklist[j+1]=tempentry;
          t=1;
       }
    } 
    debug_print("%s","sorted blocklist\n");
    for (i=0; i< blocklist_count; i++) 
        debug_print("start %d length %d\n",blocklist[i].startblock, blocklist[i].filelength);
    debug_print("%s\n","");

    /* find fist contigous block, which is large enough */
    file_start=-1;
    last_data_block=dir_start+dir_length;
    for (i=0; i< blocklist_count; i++) {
       free_space= blocklist[i].startblock - last_data_block;
       debug_print("free space %d at %d\n",free_space,last_data_block);
       if (num_blocks <= free_space) {
          file_start= last_data_block;
          break;
       }
       last_data_block= blocklist[i].startblock + blocklist[i].filelength;
    }
    if ( file_start == -1 ) {
       fprintf(stderr,"No room\n");
       free(blocklist);
       free(input_buffer);
       return(RETURN_ERROR);
    }

    debug_print("%s\n","New file");
    debug_print("dir entry at: %d\n",free_dir_entry);
    debug_print("file starts at block %d\n",file_start);
    debug_print("free space %d (blocks)\n", free_space);
    debug_print("move end of dir mark %d\n\n",extend_dir);

    /* Modify start of file in new directory entry */
    put_lif_int(new_dir_entry+12,4,file_start);

    /* Actually copy the file */ 
    debug_print("%s\n","copy file");
    lifput_file_copy(output_device,input_buffer,file_start,num_blocks);

    /* write directory record */
    debug_print("%s\n","write directory");
    lif_write_dir_entry(output_device,dir_start,free_dir_entry,new_dir_entry);
    if(extend_dir) {
       debug_print("%s\n","write end of directory mark");
       for(i=0;i<ENTRY_SIZE;i++) new_dir_entry[i]=0;
       new_dir_entry[10]= (unsigned char) 0xFF;
       new_dir_entry[11]= (unsigned char) 0xFF;
       lif_write_dir_entry(output_device,dir_start,free_dir_entry+1,new_dir_entry);
    }

    /* tidy up and quit */
    free(blocklist);
    free(input_buffer);
    if(input_file != stdin) fclose(input_file);
    lif_close(output_device);
    debug_print("%s\n","finished");
    return(RETURN_OK);
  }
