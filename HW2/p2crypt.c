/* HW2-CE-121-
 * ---UTH--- 
 * DIMITRIS KEFALAS 2533
 * GRIGORIS ILIADIS 2522
 * p2crypt is a program that crypts datas that it reads from its standar input
 * by using XOR with a key that is passed as an argument.*/
/*********** LIBRARIES & HEADERFILES ***********/
#include <fcntl.h>
#include "util.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
/*********** DEFINES ***********/
#define BLOCK 512
/*********** PROTOTYPES ***********/
int crypting (char *buff, char *code, ssize_t bytesRead,int pos);
/*********** FUNCTIONS ***********/
int main (int argc, char *argv[]){
    ssize_t bytesRead;
    char buff[BLOCK];
    int posKey;
    
    posKey=0;
    if (argc==2){
        do{
            bytesRead = myRead(STDIN_FILENO, (void *)buff, BLOCK,__LINE__,__FILE__);
            posKey=crypting(buff,argv[1],bytesRead,posKey);
            myWrite(STDOUT_FILENO,buff,bytesRead,__LINE__,__FILE__);
        }while(bytesRead!=0);
    }
    
    
    
    return (0);
}
/*Crypting datas by using XOR, by using a key*/
int crypting (char *buff, char *code, ssize_t bytesRead,int pos){
    int buffPos,keyPos;
    size_t lengthCode;
    
    
    lengthCode=strlen(code);
   
    for (buffPos=0,keyPos=pos;(ssize_t) buffPos<bytesRead;keyPos++,buffPos++){
        buff[buffPos]=buff[buffPos]^code[keyPos];
        if ((size_t)keyPos==(lengthCode-1)){
            keyPos=-1;
        }
    }
    
    return (keyPos);
}
