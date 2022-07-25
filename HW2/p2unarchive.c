/* HW2-CE-121-
 * ---UTH--- 
 * DIMITRIS KEFALAS 2533
 * GRIGORIS ILIADIS 2522
 * p2unarchive is a program that reads from its standar input some fields of a file: 
 * nameLenth, name, last access and modification time and also the size of the file,
 * and file's contents. It also creates a file according the file's fields and by 
 * copying the file's contents to the the new file that is just created.*/
/*********** LIBRARIES & HEADERFILES ***********/
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <utime.h>
#include <time.h>
#include "util.h"
/*********** DEFINES ***********/
#define BLOCK 512
/*********** PROTOTYPES ***********/
void copyFile (int fd, off_t sizeFile);
/*********** FUNCTIONS ***********/
int main (int argc, char* argv[]){  
    char *name;
    int fd;
    ssize_t bytesRead;
    time_t fileSt_atime, fileSt_mtime;
    mode_t fileSt_mode;
    size_t nameLength;
    off_t fileSize;
    struct utimbuf times;
    
    if (argc==2){
        if(mkdir(argv[1],0777)){
            fprintf (stderr, "%s @line: %d %s.\n",__FILE__,__LINE__,strerror(errno));
            exit(EXIT_FAILURE);
        }
        if(chdir(argv[1])){
            fprintf (stderr, "@line: %d %s.\n",__LINE__,strerror(errno));
            exit(EXIT_FAILURE);
         }
        do{
            bytesRead=myRead(STDIN_FILENO,(size_t*)&nameLength,sizeof(size_t),__LINE__,__FILE__);
            
            name=(char*)malloc(nameLength+1);
            if ( name == NULL){
                fprintf (stderr,"Problem in allocating memory %s: @%d.\n",__FILE__,__LINE__);
                exit (EXIT_FAILURE);
            }
            bytesRead=myRead(STDIN_FILENO,(char*)name,nameLength,__LINE__,__FILE__)+bytesRead;
            name[nameLength]='\0';
            
            bytesRead = myRead ( STDIN_FILENO, (time_t*)&fileSt_atime, sizeof(time_t), __LINE__,__FILE__) + bytesRead;
            bytesRead = myRead ( STDIN_FILENO, (time_t*)&fileSt_mtime, sizeof(time_t), __LINE__,__FILE__) + bytesRead;
            bytesRead = myRead ( STDIN_FILENO, (mode_t*)&fileSt_mode, sizeof(mode_t), __LINE__,__FILE__) + bytesRead;
            bytesRead = myRead ( STDIN_FILENO, (off_t*)&fileSize, sizeof (off_t), __LINE__,__FILE__) + bytesRead;
            
            if (bytesRead!=0){
                fd=open(name,O_CREAT | O_WRONLY,0666);
                if (fd<0){
                    fprintf (stderr, "@line: %d %s.\n",__LINE__,strerror(errno));
                    exit(EXIT_FAILURE);
                }
                copyFile(fd,fileSize);
                if (fchmod (fd,fileSt_mode)){
                    fprintf (stderr, "%s @line: %d %s.\n",__FILE__,__LINE__,strerror(errno));
                    exit (EXIT_FAILURE);
                }
                close(fd);
                
                times.actime=fileSt_atime;
                times.modtime=fileSt_mtime;
                if(utime(name,&times)){
                    perror("utime:");
                    exit(EXIT_FAILURE);
                }
            }
            free(name);
        }while(bytesRead!=0);
    }
    
    return (0);
}
/*Reading from the file its contents and writing them to its standar output.*/
void copyFile (int fd, off_t sizeFile){
    char *buff;
    int pos,how_many;
    ssize_t readBytes;
    
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
        readBytes=myRead(STDIN_FILENO,buff,how_many,__LINE__,__FILE__);
        if (pos==0){
                pos=readBytes;
        }
        else{
                pos=pos+readBytes;
        }
        myWrite(fd,buff,readBytes,__LINE__,__FILE__);
        
    
    }while (pos<sizeFile);
    free(buff);

}
    
