/*
Section CSE 3320-002
Vivek Patel
UTA ID: 1001398338

Section CSE 3320-003
Luis Estrada
UTA ID: 1001136792
*/


// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "functions.h"  
#include "ls_cd.h"
#include "volume.h"
#include <stdint.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                               // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10   // Mav shell only supports ten arguments

#define HIST_total 15 //Total number of history msh shell can show at max

FILE *fp; //To open the file

int root_address; //To get the root address

//To get the root address and store in which directory you are currently in
//helpful in "cd" function
int directory_address[1000]; 
//To take care the height of the tree. So far you can have only 1000 height of the tree
int in_main = 0; 

int history_count = 0 ;  

//Function to print history
static void print_history(char ***history)
{
  int i;
  for( i = 0; i < history_count; i++)
  {
     printf("%d: %s\n", i, (*history)[i]);
  }
}

//Manage history
//Passing command to store in history of commands
//Make sure that the history array should have only last 15 commands into it
static void manage_history(char command[], char ***history)
{
     int lm;
     if(history_count < HIST_total)
     {
         strcpy((*history)[history_count], command);       
         history_count +=1;
     }
     else
     {
          for(lm = 0; lm < (HIST_total-1); lm++)
          {
              strcpy((*history)[lm], (*history)[lm+1]);
          }
          strcpy((*history)[14], command);
          history_count = 15;
     }
}


     
int main()
{  
     //variable going to be used in for loop most of the time
     int lm = 0;
    
     //array for storing the history of the valid commands enetered by the user
     char** history = (char**) malloc(HIST_total*sizeof(char*));
      
     //Taking count of the file 
     char* file = (char*) malloc( MAX_COMMAND_SIZE );
     //nullify the allonullifycated values
     nullify(&file);

     //Nullified string to cmp whether we have file opened or not
     char* nullified = (char*) malloc( MAX_COMMAND_SIZE );
     //nullify the allocated values
     nullify(&nullified);
       
     for( lm = 0; lm<HIST_total;lm++)
     {
         history[lm] = (char*) malloc(MAX_COMMAND_SIZE * sizeof(char) );
     } 

     //Cmd_str getting the whole input from the user
     char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
     //Perfect command in command
     char * command = (char*) malloc( MAX_COMMAND_SIZE );
     //taking care of previous command
     char * last_command = (char*) malloc( MAX_COMMAND_SIZE );
     nullify(&last_command);

     while( 1 )
     {
  
        // Print out the msh prompt
        printf ("mfs> ");
       
        // Read the command from the commandline.  The
        // maximum command that will be read is MAX_COMMAND_SIZE
        // This while command will wait here until the user
        // inputs something since fgets returns NULL when there
        // is no input
        while(!fgets (command, MAX_COMMAND_SIZE, stdin));
    
        //Checking whether the user is just inputing alot of space and then press enter
        int spaces = 0;
        int count_space, counting  = 0;
        int cmd_len = (int) strlen(command);
        for( count_space = 0; count_space < cmd_len; count_space++)
        {
            int k = (int)command[count_space];
            if( k == 32) {spaces++;}
            if( k != 32 && k != 10)
            {
                counting++; break;
            }
         }
    
         // If user input does not have any characters then do not go in to the while loop to execute any commmand
        if( counting )   
        {
             //Parsing the real command after eliminating all the spaces in front
            strncpy(cmd_str, command + spaces, cmd_len);
            
            //If user want to access the history command by using '!n' command then spliting string into a integer            
            int repeat_command = 99;
            if(cmd_str[0] == '!')
              {
                  for(lm =1; lm < strlen(cmd_str); lm++)
                  {
                      if(cmd_str[lm] == ' ' || cmd_str[lm]== '\0')
                      { break;}
                      else 
                      { 
                           if((int)cmd_str[lm] > 47 && (int)cmd_str[lm] < 58)
                           {
                               if(lm == 1)
                               { 
                                   repeat_command = 0;repeat_command += (int) cmd_str[lm] - 48;
                               }
                               else
                               {
                                   repeat_command = repeat_command*10;
                                   repeat_command += (int) cmd_str[lm] - 48;
                               }
                            }
                            else {break;}
                      }
                 }
                 //If user want to acces history higher than history_count then 'command not found' will be shown to user
                  if(repeat_command < history_count)
                  {     
                       strcpy(cmd_str, history[repeat_command]);
                  }
            } 
            /* Parse input */
            char *token[MAX_NUM_ARGUMENTS];

            int   token_count = 0;                                 
                                                           
            // Pointer to point to the token
            // parsed by strsep
            char *arg_ptr;                                         
                                                           
            char *working_str  = strdup( cmd_str );                

            // we are going to move the working_str pointer so 
            // keep track of its original value so we can deallocate
            // the correct amount at the end
            char *working_root = working_str;

            // Tokenize the input stringswith whitespace used as the delimiter
            while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
               (token_count<MAX_NUM_ARGUMENTS))
              {
                 token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
                 if( strlen( token[token_count] ) == 0 )
                 {
                     token[token_count] = NULL;
                 }
                 token_count++;
              }
              
                   
              //Exit the program with status 0 if user put "exit" or "quit"
              if( !strcmp(token[0], "quit") || !strcmp(token[0], "exit"))
              {
                     return 0;
              }
               
              //Checking for open file command
              if(!strcmp(token[0], "open") || !strcmp(token[0], "Open") )
              {
                   //opening the file 
                   //First check whether the file is already opened or not
                   if( !strcmp(token[1], file) )
                   {
                       printf("Error: File system image already opened:):)\n");
                   }
                   else 
                   {
                       FILE *fpp = fopen(token[1],"r");
                       if( fpp == NULL)
                       {
                            printf("Error: File system image not found ( \"%s\").\n",token[1]);
                       }
                       else 
                       {
                            fp = fpp;
                            strcpy(file, token[1]);
                            strcpy(last_command, cmd_str);
                            manage_history(cmd_str, &history);
                       }
                   } 
              }
              else if(!strcmp(token[0], "close") || !strcmp(token[0], "close") )
              {
                   //First check whether the file is already opened or not
                   if( !strcmp(nullified, file) )
                   {
                       printf("Error: File system not open:):)\n");
                   }
                   else 
                   {
                       fclose(fp);
                       fp = NULL;
                       nullify(&file);
                       strcpy(last_command, "close");
                       manage_history(cmd_str, &history);
                   }    
              } 
              //Looking for "close" command
              else if(!strcmp(last_command, "close") && strcmp(token[0], "close") && strcmp(token[0], "open"))
              {
                       printf("Error: First system image must be opened first\n");    
              }
              //If there is no command so far then show error
              else if( !strcmp( last_command, nullified))
              {
                       printf("Error: First system image must be opened first\n");    
              }
              //Looking for "info" command which will print out the info of fat32 system
              else if( !strcmp(token[0], "info"))
              {
                    find_info(&fp);
                    manage_history(cmd_str, &history);
              }
              //Looking for "cd" command which will change the directory in fat32 system
              else if( !strcmp(token[0], "cd") )
              { 
                    if(token_count == 2)
                    {
                        root_address = find_root_address(&fp);
                        directory_address[in_main] = root_address;
                        in_main = 0;
                    }
                    else 
                    {
                       if(in_main == 0)
                       {
                           root_address = find_root_address(&fp);
                           directory_address[in_main] = root_address;
                       }
                       //Looking if user want to go in previous directory
                       if(!strcmp(token[1],"..") && in_main !=0)
                       { 
                             directory_address[in_main] = -999;
                             in_main = in_main - 1;        
                       }
                       //Looking if user is in root directory and still want to go back
                       //which is not allowed in this program
                       else if(!strcmp(token[1],"..") && in_main ==0)
                       { 
                           in_main = 0;
                           printf("You are in root direcotry can not go further back.\n");
                       }
                        else
                       {
                           //Getting the number of bytes per cluster
                           int bytes = get_clusinfo(&fp);
                           //Getting the staring cluster of the directory
                           int new_clus = get_stat(&fp, directory_address[in_main], token[1] );
                           //If direcotry not found then print error
                           if(new_clus == -999)
                           {
                                printf("Error: Unable to find directory %s\n",token[1]);
                           }
                           //Else update directory address 
                           else
                           {
                              in_main = in_main + 1;
                              directory_address[in_main] = (bytes*(new_clus - 2))+root_address;
                           }
                         //   printf("%d The address is \n", directory_address[in_main]);
                        }
                     }
                     manage_history(cmd_str, &history);
              }
              else if( !strcmp(token[0], "read") )
              { 
                    if(token_count == 5)
                    {
                      if(in_main == 0)
                      {
                          root_address = find_root_address(&fp);
                          directory_address[in_main] = root_address;
                      }
                      int bytes = get_clusinfo(&fp);
                      //Getting the staring cluster of the file
                      int new_clus = data_stat(&fp, directory_address[in_main], token[1] );
                      //If direcotry not found then print error
                      if(new_clus == -999)
                      {
                           printf("Error: Unable to read file \"%s\"\n",token[1]);
                      }
                      // Else try to read file
                      else
                      {
                           int read_address = (bytes*(new_clus - 2))+root_address;
                           //Getting the offsets
                           int start = convert_int(token[2]);
                           int end = convert_int(token[3]);
                           //If offsets are in wrong format then show error
                           if(start == -999 || end == -999)
                           { printf("Error: Enter offsets in correct format.\n"); }
                           else if( start > end){ printf("Error: Enter offsets in correct format.\n");}
                           else
                           {
                              int len = end -start;
                              //reading the file in decimal format
                              char a;
                              int i = 0;
                              fseek( fp, read_address+start, SEEK_SET);
                                 
                              for( i = 0; i<len;i++)
                              { 
                                 fread( &a, 1,1, fp);
                                 printf("%d ",a);
                                
                              }
                              printf("\n");
                           }
                        }
                     manage_history(cmd_str, &history);
                     }
                     else {printf("Error: Please use right format.\n");} 
              }
              else if( !strcmp(token[0], "ls") )
              {
                    if(in_main == 0)
                    {
                      root_address = find_root_address(&fp);
                      directory_address[in_main] = root_address;
                    }
                    //Use ls function to list out the file in current directory
                    ls_function(&fp, directory_address[in_main]);
                    manage_history(cmd_str, &history);
              }
	      else if( !strcmp(token[0], "stat"))
              {
                    if(in_main == 0)
                    {
                      root_address = find_root_address(&fp);
                      directory_address[in_main] = root_address;
                    }
                    //Getting the stat of file based on the current directory
                    //First look for file, if file is in the current directory then print the stat 
                    //Else show error
                   // printf("%d The address is \n", directory_address[in_main]);
                    int found = stat_function(&fp,directory_address[in_main], token[1]);
                    if(found == 0)
                    {
                         printf("Error: File not found %s.\n",token[1]);
                    }
                    manage_history(cmd_str, &history);   
              }
              else if( !strcmp(token[0], "volume"))
              {
                  //Print the volume info of the given file
                  print_vol(&fp);
                  manage_history(cmd_str, &history);
              }
              else if( !strcmp(token[0], "history"))
              {    
                    manage_history("history", &history);
                    print_history(&history); 
              }
              else if(!strcmp(token[0], "get"))
              {
                  if(in_main == 0)
                  {
                      root_address = find_root_address(&fp);
                      directory_address[in_main] = root_address;
                  } 
                  int bytes = get_clusinfo(&fp);
                  int new_clus = data_stat(&fp, directory_address[in_main], token[1] );
                  if(new_clus == -999)
                  {
                         printf("Error: Unable to find the file \"%s\"\n",token[1]);
                  }
                  else
                  {
                        int read_address = (bytes*(new_clus - 2))+root_address;
                        int len = file_size_stat(&fp, directory_address[in_main], token[1] );
                        char a[len];
                        FILE* work = fopen(token[1], "wb");
                        fseek( fp, read_address, SEEK_SET);   
                        fread(&a, len,1,fp);
                        fwrite(a,len,1,work);   
                        fclose(work);
                  }   
              }
              else {printf("Error: command not found\n");}
              
              
               free( working_root );
           }      
       nullify(&cmd_str) ;
  }
  free(cmd_str);
  free(command);
  return 0;
}
