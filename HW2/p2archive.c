/* HW2-CE-121-
 * ---UTH--- 
 * DIMITRIS KEFALAS 2533
 * GRIGORIS ILIADIS 2522
 * p2unarchive is a program that reads from its standar input a name file series.
 * It creates an archive in which fields like the name length(binary), the name (asci),
 * the last modification and access time (binary), permissions, the size and the contens 
 * of a file are saved in.*/ 
/*********** LIBRARIES & HEADERFILES ***********/
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/stat.h>
#include "util.h"
/*********** DEFINES ***********/
#define FILEPATH_SIZE 4056
#define BLOCK 512
/*********** PROTYPES ***********/
int myOpen (const char *name,int line);
void contentsFile(int fd,char *path);
void copyFile (int fd, off_t size);
/*********** FUNCTIONS ***********/
int main (void){
    int numberInput, fileDescriptor;
    char buff[FILEPATH_SIZE];
    char format [10];

    sprintf (format,"%%%ds",FILEPATH_SIZE-1);
    do{
        numberInput = scanf (format,buff);
        if (numberInput==EOF){
            break;
        }
        fileDescriptor = myOpen ( buff, __LINE__);
        if (fileDescriptor != -1){
            contentsFile ( fileDescriptor, buff);
        }
    }while(1);
    

    return(0);
}
int myOpen (const char *name,int line){
    int fileDescriptor;
    
    fileDescriptor=open(name,O_RDONLY);
    if ( fileDescriptor < 0){
    /*In p2archive we want to read the contens of a file. If a file is not readable.
     *Then errno will have the value of EACCES. The open sc will fail, by checking 
     *the errno's value we ensure that p2archive will not be finished if one file 
     *in the directory is not readable.*/ 
	if ( errno != EACCES){
            fprintf ( stderr, "./%s @line %d:", __FILE__,line);
            fprintf ( stderr, " %s. %s\n", strerror(errno), name);
            exit (EXIT_FAILURE);
        }
    }
    
    return (fileDescriptor);
}
/*Writing to the standar output the main fields of the file, like: nameLength,name, 
 *last access time, last modification time, permissions, the size of the file.*/
void contentsFile(int fd,char *path){
    char *name;
    size_t nameLength;
    struct stat infoFile;
    
    name=basename(path);
    nameLength=strlen(name);
    myWrite(STDOUT_FILENO,(size_t*)&nameLength,sizeof(size_t),__LINE__,__FILE__);
    myWrite(STDOUT_FILENO,(char*)name,nameLength,__LINE__,__FILE__);
    if (fstat(fd,&infoFile)){
        fprintf (stderr, "@line %d:",__LINE__-1);
        fprintf (stderr, " %s.\n",strerror(errno));
        exit(EXIT_FAILURE);
    }
    myWrite(STDOUT_FILENO,(time_t*)&(infoFile.st_atime),sizeof(time_t),__LINE__,__FILE__);
    myWrite(STDOUT_FILENO,(time_t*)&(infoFile.st_mtime),sizeof(time_t),__LINE__,__FILE__);
    myWrite(STDOUT_FILENO,(mode_t*)&(infoFile.st_mode),sizeof(mode_t),__LINE__,__FILE__);
    myWrite(STDOUT_FILENO,(off_t*)&(infoFile.st_size),sizeof(off_t),__LINE__,__FILE__);
    copyFile(fd,infoFile.st_size);
    if(close(fd)){
	perror("close:");
	exit(EXIT_FAILURE);
    }
}
/*Reading from the file its contents and writing them to its standar output.*/
void copyFile (int fd, off_t sizeFile){
    char *buff;
    int pos,how_many;
    ssize_t readBytes,bytesWritten;
    
    pos=0;
    
    buff = (char*) malloc (BLOCK);
    if (buff==NULL){
	fprintf(stderr,"Cannot allocate memory!\n");
	exit(EXIT_FAILURE);
    }
    do{
        if ((sizeFile-pos)>=BLOCK){
            how_many=BLOCK;
        }
        else{
            how_many=sizeFile-pos;
        }
        readBytes=myRead(fd,buff,how_many,__LINE__,__FILE__);
        if (pos==0){
                pos=readBytes;
        }
        else{
                pos=pos+readBytes;
        }
        bytesWritten=myWrite(STDOUT_FILENO,buff,readBytes,__LINE__,__FILE__);
        
    }while (bytesWritten!=0);
   free(buff); 
}
   
