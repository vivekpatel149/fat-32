#include<string.h>
#include<stdio.h>
#include<stdint.h>

#define MAX_COMMAND_SIZE 255 

uint16_t  BPB_BytesPerSec;
uint8_t  BPB_SecPerClus;
uint16_t  BPB_RsvdseCnt;
uint8_t  BPB_NumFATS;
uint32_t BPB_FATSz32;



static void nullify(char **file)
{
    int i = 0;
    for( i = 0; i < MAX_COMMAND_SIZE; i++)
    {
      (*file)[i] = '\0';
    } 
}

//Function to get the info of the System image of FAT 32 file system. 
static void information(FILE **fp)
{
   //seek till the offset i.e. 11 and then read from there for 2 bytes
   //reading bytes per sector
   fseek( (*fp), 11, SEEK_SET);
   fread( &BPB_BytesPerSec, 2, 1, (*fp));
     
   //seek till the offset i.e. 13 and then read from there for 1 bytes
   //reading sector per cluster
   fseek( (*fp), 13, SEEK_SET);
   fread( &BPB_SecPerClus, 1, 1, (*fp));

   //seek till the offset i.e. 14 and then read from there for 2 bytes
   //reading Rsvdse Count
   fseek( (*fp), 14, SEEK_SET);
   fread( &BPB_RsvdseCnt, 2, 1, (*fp));

   //seek till the offset i.e. 16 and then read from there for 1 bytes
   //reading Number of FATS
   fseek( (*fp), 16, SEEK_SET);
   fread( &BPB_NumFATS, 1, 1, (*fp));
   
   //seek till the offset i.e. 36 and then read from there for 4 bytes
   //reading FAT size
   fseek( (*fp), 36, SEEK_SET);
   fread( &BPB_FATSz32, 4, 1, (*fp));
}
 
//It will print Bytes per sector , Sector per cluster, Rserved sector count, Number of FTAs, and FAT size
//It will use info function
static void find_info(FILE **fp)
{  
   information(fp); 
   //printting all values

   printf("BPB_BytesPerSec: %d\n",BPB_BytesPerSec); 
   printf("BPB_BytesPerSec: %x\n\n",BPB_BytesPerSec); 

   printf("BPB_SecPerClus:  %d\n",BPB_SecPerClus);  
   printf("BPB_SecPerClus:  %x\n\n",BPB_SecPerClus); 

   printf("BPB_RsvdseCnt:   %d\n",BPB_RsvdseCnt); 
   printf("BPB_RsvdseCnt:   %x\n\n",BPB_RsvdseCnt);

   printf("BPB_NumFATS:     %d\n",BPB_NumFATS);
   printf("BPB_NumFATS:     %x\n\n",BPB_NumFATS);

   printf("BPB_FATSz32:     %d\n",BPB_FATSz32);
   printf("BPB_FATSz32:     %x\n\n",BPB_FATSz32);

}

//find the first root address
//it will use find info function then get address 
static int find_root_address(FILE **fp)
{
  information(fp);
  int address = ( BPB_NumFATS * BPB_FATSz32 * BPB_BytesPerSec) + (BPB_RsvdseCnt * BPB_BytesPerSec);
  return address;
}

//This function will give you the Bytespersector * sector per cluster
static int get_clusinfo(FILE **fp)
{
  information(fp);
  int bytes = BPB_BytesPerSec * BPB_SecPerClus;
  return bytes;
}

static int convert_int(char string[])
{
     int len = strlen(string);
     int a = 0 ;
     int i = 0;
     int lm = 1;
     for( i = 0; i< len; i++)
     {
         int char_value =(int)string[i]; 
         if( char_value > 47 && char_value < 58)
         {
                if(lm == 1){ a += char_value - 48;lm++;}
                else
                {
                     a = a*10;
                     a = a + char_value - 48;
                 }
         }
         else {return -999;}
     }
     return a;
}

/*
 *Parameters: The current sector number that points to a block of data
 *Returns: The value of the address for that block of data
 *Description: Finds the starting address of a block of data given the sector number
 *corresponding to that data block
*/
int LBAToOffset(int32_t sector)
{
    return((sector-2) * BPB_BytesPerSec) + (BPB_BytesPerSec * BPB_RsvdseCnt) + (BPB_NumFATS * BPB_FATSz32 * BPB_BytesPerSec);
}

/*
*Purpose: Given the logical block address, look up into the first FAT and return the logical
*block addresss of the block in the file. If there is not further block then return -1
*/
int16_t NextLB(uint32_t sector, FILE **fp)
{
    uint32_t FATAddress = (BPB_BytesPerSec * BPB_RsvdseCnt) + (sector * 4);
    int16_t val;
    fseek( (*fp), FATAddress, SEEK_SET);
    fread(&val, 2, 1, (*fp));
    return val;
}

