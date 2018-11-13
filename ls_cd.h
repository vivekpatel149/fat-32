#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

//#include "functions.h"

struct DirectoryEntry
{
    char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t Unused1[8];
    uint16_t DIR_FirstClusterHigh;
    uint8_t Unused2[4];
    uint16_t DIR_FirstClusterLow;
    uint32_t DIR_FileSize;
};

struct DirectoryEntry Dir[16];
struct DirectoryEntry Sub_Dir[16];

/*Clean the whole string*/
static void clean(char* string, int len)
{
    int k = 0;
    for( k = 0; k < len; k++)
    {
        string[k] = '\0';
    }
}
/*printing the information of the specified direcotry index*/
static void print_info(int i)
{
   // printf("Name:                          %s\n", Dir[i].DIR_Name);
    printf("File Attribute\t\tSize\t\tStarting Cluster Number\n%d\t\t\t%d\t\t%d\n", Dir[i].DIR_Attr, Dir[i].DIR_FileSize, Dir[i].DIR_FirstClusterLow);
}

//This function remove the space and make string clean
static void remove_space(char* string)
{
   int len = strlen(string);
   char* change = (char*) malloc(sizeof(char)*len);
   int i = 0; 
   int j = 0;
   for( i = 0; i < len; i++)
   {
      int char_value = (int)(string[i]);
      if( char_value != 32 )
      {
           change[j] = string[i];
           j++;
      }
   }
   for( j = j; j < len; j++) { change[j] = '\0'; }
   strcpy(string, change);
   free(change);
}

//This fucntion makes all the character of string upper case
static void make_upper(char* string)
{
   int len = strlen(string);
   int i = 0;
   for( i = 0; i < len; i++)
   {
      int char_value = (int)(string[i]);
      if( char_value > 96 && char_value < 123 )
      {
          string[i] = (char)(char_value-32);
      }
   }
}

//This function split the string 
static void split(char* string,char* name, char* ext)
{
     int i = 0; int j = -99;
     int len = strlen(string);
     
     for(i = 0; i < len; i++)
     {
          if(string[i] == '.')
          {
              j = i; break;
          }
          else 
          { 
              name[i] = string[i]; 
          }
     } 
     if ( j != -99)  
     {
         i = 0;  
         for( j = j+1; j < len; j++)
         {   
              if( i == 3) {break;}
              else { ext[i] = string[j]; i++;}
         }
    }
}

//This function split the string 
static void split_dir(char* string,char* name,char* ext)
{
     int i = 0; int j = 8;
     strncpy(name,string,8);
     for( j = 8; j < 11; j++)
     {   
          ext[i] = string[j]; i++;
     } 
}

/* This function will fill the entries of the direcotry */
static void fill_directories(FILE **fp,int address)
{
    int i = 0;
    for(i = 0; i < 16; i++)
    {
        fseek( (*fp), address, SEEK_SET);
        fread( Dir[i].DIR_Name, 11, 1, (*fp));
        
        fseek( (*fp), (address+0x0B), SEEK_SET);
        fread( (&Dir[i].DIR_Attr), 1, 1, (*fp));
        
        fseek( (*fp), (address+0x14), SEEK_SET);
        fread( &(Dir[i].DIR_FirstClusterHigh), 2, 1, (*fp));
   
        fseek( (*fp), (address+0x1A), SEEK_SET);
        fread( &(Dir[i].DIR_FirstClusterLow), 2, 1, (*fp));
   
        fseek( (*fp), (address+0x1C), SEEK_SET);
        fread( &(Dir[i].DIR_FileSize), 4, 1, (*fp));
 
        address = address+(int)sizeof(struct DirectoryEntry);
      }   
}

/*  This fucntion is going to print the valid available directory in the file  */
static void ls_function(FILE **fp, int address)
{
    fill_directories(fp, address);
     
    int i = 0 ;
    for( i = 0; i<16; i++)
    {
        if( Dir[i].DIR_Attr == 0x01 ||  Dir[i].DIR_Attr == 0x10 || Dir[i].DIR_Attr == 0x20)
        { 

                if( (int)(Dir[i].DIR_Name[0]) != 0xffffffe5 && (int)(Dir[i].DIR_Name[0]) != 0xffffff05)
                {
                     if( Dir[i].DIR_Attr == 0x10)
                     {
                          char* a = (char*) malloc(sizeof(char) * 8);
                          clean(a,8);                        
                          strncpy(a,Dir[i].DIR_Name,8);
                          printf("%s\n",a);
                          free(a);
                     }
                     else
                     {
                          printf("%s\n",Dir[i].DIR_Name);         
                     }
                }
        }    
    }     
     
}

/* This is stat function to show the stats of the directory */
static int stat_function(FILE **fp,int address,char stat[])
{
     fill_directories(fp, address);
    // printf("%d The sent address is \n", address);
     int found = 0;
     /*making stat string in uppercase*/
     
     int i = 0 ; 

     make_upper(stat);

     char* ext = (char*) malloc(sizeof(char) * 3);
     char* stat_name = (char*) malloc(sizeof(char) * 8);
     
     clean(stat_name, 8);
     clean(ext, 3);
     split(stat,stat_name,ext);
  
     char* d_ext = (char*) malloc(sizeof(char) * 3);
     char* d_name = (char*) malloc(sizeof(char) * 8);

     for( i =0; i < 16; i++)
     {
           
           clean(d_name, 8);
           clean(d_ext, 3);
     
           make_upper(Dir[i].DIR_Name);

           split_dir(Dir[i].DIR_Name, d_name, d_ext);
          
           remove_space(d_name);
           
           if( Dir[i].DIR_Attr == 0x10 )
           {
           //    printf("%s stat_name, %s d_name\n", stat_name, d_name);
               if(!strcmp(stat_name,d_name))
               {
                  found = 1;
                  print_info(i);
                  break;
               }           
           }
           else if( !strcmp(stat_name,d_name) && !strcmp(ext,d_ext))
           {
               //printf("%s stat_name,%s ext, %s d_name, %s dext\n", stat_name,ext , d_name, d_ext);
               found = 1;
               print_info(i);
               break;
           }
           printf("\n");
     }
     free(ext);
     free(stat_name);
     free(d_name);
     free(d_ext);          
     return found;
}

//Time to find the directory
//This function will use the same startegy as "stat_function" but instead of printing out
//info of the direcotry it will return the starting cluster number of the directory
//It will only compare string with directories not with file name.
//It is given that each directory has "0x10" as an attribute
//If directory not found then it will return -999, which is an error signal in this program

static int get_stat(FILE** fp, int address,char string[])
{
    // printf("%d The address sent is \n", address);
     fill_directories(fp, address);
   
     int found = -999;
     /*making stat string in uppercase*/
     
     int i = 0 ; 
     make_upper(string);
     char* ext = (char*) malloc(sizeof(char) * 3);
     char* stat_name = (char*) malloc(sizeof(char) * 8);
     
     clean(stat_name, 8);
     clean(ext, 3);
     split(string,stat_name,ext);
  
     char* d_ext = (char*) malloc(sizeof(char) * 3);
     char* d_name = (char*) malloc(sizeof(char) * 8);

     for( i =0; i < 16; i++)
     {         
           if( Dir[i].DIR_Attr == 0x10 )
           {
               clean(d_name, 8);
               clean(d_ext, 3);
     
               make_upper(Dir[i].DIR_Name);
 
               split_dir(Dir[i].DIR_Name, d_name, d_ext);
          
               remove_space(d_name);
               if(!strcmp(stat_name,d_name))
               {
                  found = Dir[i].DIR_FirstClusterLow;
                  break;
               }           
           }
     }
     free(ext);
     free(stat_name);
     free(d_name);
     free(d_ext);          
     return found;
}

//Time to read some data from txt files
//This function will use the same startegy as "stat_function" but instead of printing out
//info of the file it will return the starting cluster number of the file
//It will only compare string with only files not with directories name.
//It is given that each directory has "0x10" as an attribute
//If file not found then it will return -999, which is an error signal in this program

static int data_stat(FILE **fp,int address,char string[])
{
     
     fill_directories(fp, address);
   
     int found = -999;
     /*making stat string in uppercase*/
     
     int i = 0 ; 
     make_upper(string);
     char* ext = (char*) malloc(sizeof(char) * 3);
     char* stat_name = (char*) malloc(sizeof(char) * 8);
     
     clean(stat_name, 8);
     clean(ext, 3);
     split(string,stat_name,ext);
  
     char* d_ext = (char*) malloc(sizeof(char) * 3);
     char* d_name = (char*) malloc(sizeof(char) * 8);

     for( i =0; i < 16; i++)
     {         
           if( Dir[i].DIR_Attr != 0x10 )
           {
               clean(d_name, 8);
               clean(d_ext, 3);
     
               make_upper(Dir[i].DIR_Name);
 
               split_dir(Dir[i].DIR_Name, d_name, d_ext);
          
               remove_space(d_name);
               if(!strcmp(stat_name,d_name) && !strcmp(ext,d_ext))
               {
                  found = Dir[i].DIR_FirstClusterLow;
                  break;
               }           
           }
     }
     free(ext);
     free(stat_name);
     free(d_name);
     free(d_ext);          
     return found;
}

static int file_size_stat(FILE **fp,int address,char string[])
{

     fill_directories(fp, address);
   
     int found = -999;
     /*making stat string in uppercase*/
     
     int i = 0 ; 
     make_upper(string);
     char* ext = (char*) malloc(sizeof(char) * 3);
     char* stat_name = (char*) malloc(sizeof(char) * 8);
     
     clean(stat_name, 8);
     clean(ext, 3);
     split(string,stat_name,ext);
  
     char* d_ext = (char*) malloc(sizeof(char) * 3);
     char* d_name = (char*) malloc(sizeof(char) * 8);

     for( i =0; i < 16; i++)
     {         
           if( Dir[i].DIR_Attr != 0x10 )
           {
               clean(d_name, 8);
               clean(d_ext, 3);
     
               make_upper(Dir[i].DIR_Name);
 
               split_dir(Dir[i].DIR_Name, d_name, d_ext);
          
               remove_space(d_name);
               if(!strcmp(stat_name,d_name) && !strcmp(ext,d_ext) )
               {
                  found = Dir[i].DIR_FileSize;
                  break;
               }           
           }
     }
     free(ext);
     free(stat_name);
     free(d_name);
     free(d_ext);          
     return found;
}
