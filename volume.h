#include <string.h>
#include <stdio.h>
#include <stdint.h>

char BS_VolLab[11];

//Function to get the Volume Label
static void vol(FILE **fp)
{
    //seek until the offset i.e. 71 and then read from there for 11 bytes
    //reading volume label
    fseek( (*fp), 71, SEEK_SET);
    fread( &BS_VolLab, 11, 1, (*fp));
}

//Function to print the Volume Label
static void print_vol(FILE **fp)
{
    vol(fp);
    
    //printting all values
    printf("Volume name of the file is  %s \n",BS_VolLab);
}
