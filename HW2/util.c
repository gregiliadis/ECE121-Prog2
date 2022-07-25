#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

ssize_t myWrite(int fd, const void *buff, size_t count,int line,char *file){
    ssize_t counter, bytesWritten;
    
    counter=0;
    do{
        bytesWritten=write(fd,(void*)buff+counter,count);
        if (bytesWritten<0){
            fprintf (stderr, "%s @line %d:",file,line);
            fprintf (stderr, " %s.\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
        counter=counter+bytesWritten;
        count=count-bytesWritten;
    }while (count>0);
    
    return (counter);
}
ssize_t myRead(int fd, const void *buff, size_t count,int line, char *file){
    ssize_t counter, bytesRead;
    
    counter=0;
    do{
        bytesRead=read(fd,(void*)buff+counter,count);
        if (bytesRead<0){
            fprintf (stderr, "%s @line %d:",file,line);
            fprintf (stderr, " %s.\n",strerror(errno));
            exit(EXIT_FAILURE);
        }
        counter=counter+bytesRead;
        count=count-bytesRead;
    }while (count>0 && bytesRead!=0);
    
    return (counter);
}
