/* HW2-CE-121-
 * ---UTH--- 
 * DIMITRIS KEFALAS 2533
 * GRIGORIS ILIADIS 2522
 * A program that according some arguments (eg:./hw2 -(E/D) nameFolder keyCode archive),
 * if case is -E, it calls some procceses in order to store the readable files of a file of in  
 * an archive. If case is -D, it calls some processes in order to create a Directory and store in 
 * the files that the archive contains.*/

/*********** LIBRARIES & HEADER FILES ***********/
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include "util.h"
/*********** PROTOTYPES ***********/
void codingMagicNumber (char magicNumber[]);
pid_t myfork (char *string,int line); 
void myPipe (int fd[],int line);
int mydup2 (int fd1, int fd2, int line);
void myclose (int fd, int line);
int processTwo (char *folder,int line);
int processThree (int line);
int crypting (int line, char str[]);
int subprocessesCaseE (int fileDescriptor,char *str2, char *str3, char *filePath);
int subprocessesCaseD (int fileDescriptor,char *str2, char *str3);
int unarchiving(int line, char *folder);
void killingProcesses (pid_t pidChild, pid_t p2id, pid_t p3id, pid_t p4id, char *filepath);
void childThatFinished (pid_t pidChild, pid_t p2id, pid_t p3id, pid_t p4id,int *childFinished);
void closingPipes (int pipe1[], int pipe2[], int fileDescriptor);
/*********** FUNCTION'S BODIES ***********/
int main(int argc, char* argv[]){
    char magicNumber[10],*buff;
    int fileDescriptor;
    
     if (argc==5){
        codingMagicNumber(magicNumber);
        if (!strcmp(argv[1],"-E")){
            fileDescriptor=open(argv[4],O_CREAT | O_EXCL | O_WRONLY,0666);
            if (fileDescriptor<0){
                fprintf (stderr, "%s @line %d: %s\n",__FILE__,__LINE__,strerror(errno));
                exit(-1);
            }
            myWrite(fileDescriptor,magicNumber,strlen(magicNumber),__LINE__,__FILE__);
            subprocessesCaseE (fileDescriptor,argv[2],argv[3],argv[4]);
        }
        if (!strcmp(argv[1],"-D")){
            fileDescriptor=open(argv[4],O_RDONLY);
            if (fileDescriptor<0){
                fprintf (stderr, "%s @line %d: %s\n",__FILE__,__LINE__,strerror(errno));
                exit(-1);
            }
            buff=(char*) malloc (strlen(magicNumber)+1);
            if (buff==NULL){
                fprintf(stderr, "Problem in allocating memory! @%d on %s\n",__LINE__,__FILE__);
                exit(EXIT_FAILURE);
            }
            myRead(fileDescriptor,buff,strlen(magicNumber),__LINE__,__FILE__);
            buff[strlen(magicNumber)]='\0';
            if (strcmp(buff,magicNumber)){
                fprintf(stderr,"Not a crypted file.\n");
                exit(EXIT_FAILURE);
            }
            free(buff);
            subprocessesCaseD (fileDescriptor,argv[2], argv[3]);
          }
      }

    
    return(0);
}
void codingMagicNumber (char magicNumber[]){
    
    strcpy(magicNumber,"P2CRYPTAR");
    
}
/*Wrapper fork.*/
pid_t myfork(char *string,int line){
    pid_t child;
    
    child=fork();
    if (child==-1){
        fprintf (stderr,"%s @ line %d: %s\n",string,line,strerror(errno));
     	 exit(-1);
    }
    
    return(child);
}
/*Wrapper Pipe.*/
void myPipe(int fd[],int line){
    
    if(pipe(fd)==-1){
        fprintf (stderr,"%s @ line %d: %s\n",__FILE__,line,strerror(errno));
        exit(EXIT_FAILURE);
    }
}
/*Wrapper dup2.*/    
int mydup2(int fd1, int fd2, int line){
    int new_fd;
    
    if ((new_fd=dup2(fd1,fd2))==-1){
        fprintf (stderr,"%s @ line %d: %s\n",__FILE__,line,strerror(errno));
        exit(-1);
    }
    
    return(new_fd);
}
/*Wrapper close.*/
void myclose(int fd, int line){
    
    if (close(fd)){
        fprintf (stderr,"%s @ line %d: %s\n",__FILE__,line,strerror(errno));
        exit(-1);
    }
    
}
/*Subprocesse case Encrypt, creating an archive file.*/
int subprocessesCaseE (int fileDescriptor, char *str2, char *str3, char *filepath){
    int pipe1[2],pipe2[2];
    pid_t p2id,p3id,p4id,pidChild; 
    int status, finishedPid, counter;
    myPipe(pipe1,__LINE__);

    finishedPid=0;
    /*Creating a Child process dirlist.*/
    p2id=myfork(__FILE__,__LINE__);
    if (p2id==0){
	/*Closing unusable fileDescriptor, ex. the read pipe end.*/
        myclose(pipe1[0],__LINE__);
	myclose(fileDescriptor,__LINE__);
        /*Redirecting the STDOUT to the pipe write end.*/
        mydup2(pipe1[1],STDOUT_FILENO,__LINE__);
        myclose(pipe1[1],__LINE__);
        
        processTwo(str2,__LINE__);
        return(1);
    }
    else{
        myPipe(pipe2,__LINE__);
 	/*Creating a Child process p2archive*/
        p3id=myfork(__FILE__,__LINE__);
        if(p3id==0){
            
	/*Closing unusable fileDescriptor, ex. the write pipe1 end, read pipe2 end.*/
	    myclose(pipe1[1],__LINE__);
            mydup2(pipe1[0],STDIN_FILENO,__LINE__);
            myclose(pipe1[0],__LINE__);
	    myclose(fileDescriptor,__LINE__);
        /*Closing unusable fileDescriptor, ex. the read pipe end.*/
            mydup2(pipe2[1],STDOUT_FILENO,__LINE__);
            myclose(pipe2[0],__LINE__);
            myclose(pipe2[1],__LINE__);
            
            processThree(__LINE__);
            
            return(1);
        }
        else{
            p4id=myfork(__FILE__,__LINE__);
            if (p4id==0){
	/*Closing unusable fileDescriptor, ex. the read pipe end.*/
	        myclose(pipe1[0],__LINE__);
                myclose(pipe1[1],__LINE__);
                mydup2(pipe2[0],STDIN_FILENO,__LINE__);
                myclose(pipe2[0],__LINE__);
                mydup2(fileDescriptor,STDOUT_FILENO,__LINE__);
                myclose(fileDescriptor,__LINE__);
                myclose(pipe2[1],__LINE__);
                crypting(__LINE__,str3);
                
                return(2);
            }
        }
    }

    /*Closing unusable fileDescriptor, ex. the read pipe end.*/
    closingPipes (pipe1,pipe2,fileDescriptor);
    for (counter=0;counter<3;counter++){
        if ((pidChild=waitpid (-1,&status,WUNTRACED)) == -1){
            fprintf (stderr, "%s @line %d: %s\n",__FILE__,__LINE__,strerror(errno));
            exit (EXIT_FAILURE);
        }
        else{
	    childThatFinished (pidChild, p2id, p3id, p4id,&finishedPid);
            if(WEXITSTATUS(status)==EXIT_FAILURE){
                killingProcesses(finishedPid,p2id,p3id,p4id,filepath);
            }
        }
    }
        
    return (0);
   
}
/*Process dirlist.*/
int processTwo(char *folder,int line){
    
     if(execlp("./dirlist","dirlist",folder,NULL)==-1){
        fprintf (stderr,"%s @ line %d: %s\n",__FILE__,line,strerror(errno));
        exit(-1);
     }
     else{
         fprintf (stderr,"end of dirilstiiiiing\n");
     }
    
    return(1);
}
/*Process p2archive.*/
int processThree(int line){
        
     if(execlp("./p2archive","p2archive",NULL)==-1){
        fprintf (stderr,"%s @ line %d: %s\n",__FILE__,line,strerror(errno));
        exit(-1);
    }
    
    return(1);
}
/*Process: p2crypt.*/
int crypting(int line,char str[] ){
        
     if(execlp("./p2crypt","./p2crypt",str,NULL)==-1){
        fprintf (stderr,"%s @ line %d: %s\n",__FILE__,line,strerror(errno));
        exit(-1);
    }
    
    return(1);
}
/*Subprocesse case Decrypt, creating a folder, decoding an archive file.*/
int subprocessesCaseD (int fileDescriptor,char *str2, char *str3){
    int fpipe1[2];
    pid_t p2id,p3id,pidChild;
    int status;
    
    myPipe(fpipe1,__LINE__);
            
    p2id=myfork(__FILE__,__LINE__);
    /*Creating a Child process p2crypt*/
    if (p2id==0){
	/*Closing unusable fileDescriptor, ex. the write pipe1 end, read pipe2 end.*/
	mydup2(fileDescriptor,STDIN_FILENO,__LINE__);
        myclose(fileDescriptor,__LINE__);
        myclose(fpipe1[0],__LINE__);
        mydup2(fpipe1[1],STDOUT_FILENO,__LINE__);
        myclose(fpipe1[1],__LINE__);
        crypting(__LINE__,str3);
        return(1);
    }
    else{
	/*Creating a Child process p2archive*/
        p3id=myfork(__FILE__,__LINE__);
        if (p3id==0){
	/*Closing unusable fileDescriptor, ex. the write pipe1 end, read pipe2 end.*/
            myclose(fpipe1[1],__LINE__);
            mydup2(fpipe1[0],STDIN_FILENO,__LINE__);
            myclose(fpipe1[0],__LINE__);
            
            unarchiving (__LINE__,str2);
            return(1);
        }
    }
    
    /*Closing unusable fileDescriptor, ex. the write fpipe1 end, read fpipe2 end.*/
    myclose(fpipe1[0],__LINE__);
    myclose(fpipe1[1],__LINE__);
    myclose(fileDescriptor,__LINE__);
    if ((pidChild=waitpid (-1,&status,WUNTRACED)) == -1){
        fprintf (stderr, "%s @line %d: %s\n",__FILE__,__LINE__,strerror(errno));
        exit (EXIT_FAILURE);
    }
    else{
        if(WEXITSTATUS(status)==EXIT_FAILURE){
            pidChild == p3id ? kill(p2id,SIGKILL) : kill(p3id,SIGKILL);
            exit (EXIT_FAILURE);
        }
    }
    if((pidChild=waitpid(-1,NULL,0))==-1){
        fprintf (stderr, "%s @line %d: %s\n",__FILE__,__LINE__,strerror(errno));
        exit (EXIT_FAILURE);
    }
    
    return (0);
}
/*Procces p2unarchive.*/
int unarchiving(int line, char *folder){
   
    if(execlp("./p2unarchive","p2unarchive",folder,NULL)==-1){
        fprintf (stderr,"%s @ line %d: %s\n",__FILE__,line,strerror(errno));
        exit(EXIT_FAILURE);
     }
    
    return(1);
}
/*Closing pipes in main process in Encryption.*/
void closingPipes (int pipe1[], int pipe2[], int fileDescriptor){
    
        myclose(pipe1[1],__LINE__);
        myclose(pipe1[0],__LINE__);
        myclose(pipe2[1],__LINE__);
        myclose(pipe2[0],__LINE__);
	myclose(fileDescriptor,__LINE__);
    
    return;
}
/*Killing the Processes.*/
void killingProcesses (int finishedPid, pid_t p2id, pid_t p3id, pid_t p4id, char *filepath){
    
    if (remove (filepath)){
        fprintf(stderr,"%s @%d %s.\n",__FILE__,__LINE__,strerror(errno));
        exit(EXIT_FAILURE);
    }

   /*If finishedPid == 2 ---> p2id EXIT_FAILURE.*/     
    if (finishedPid==2){
        kill(p3id,SIGKILL);
        kill(p4id,SIGKILL);
    }
   /*If finishedPid == 3 ---> p3id EXIT_FAILURE.*/     
    
    if (finishedPid==3){
        kill(p2id,SIGKILL);
        kill(p4id,SIGKILL);
    }
    
   /*If finishedPid == 4 ---> p4id EXIT_FAILURE.*/     
    if (finishedPid==4){
        kill(p2id,SIGKILL);
        kill(p3id,SIGKILL);
    }
   /*If finishedPid == 5 ---> p2id | p3id EXIT_FAILURE.*/     
    
    if (finishedPid==5){
        kill(p4id,SIGKILL);
    }
    
   /*If finishedPid == 6 ---> p2id | p4id  EXIT_FAILURE.*/     
    if (finishedPid==6){
        kill(p3id,SIGKILL);
    }
    
   /*If finishedPid == 7 ---> p3id | p4id  EXIT_FAILURE.*/     
    if (finishedPid==7){
        kill(p2id,SIGKILL);
    }
    
    exit(EXIT_FAILURE);
}
/*If one process has been exited we have to check which process has been terminated. This is happening by using the 
 *a number that is called finishedPid. */    
void childThatFinished (pid_t pidChild, pid_t p2id, pid_t p3id, pid_t p4id, int *finishedPid){    
    if (pidChild==p2id){
        (*finishedPid)=(*finishedPid)+2;
    }
    if (pidChild==p3id){
        (*finishedPid)=(*finishedPid)+3;
    }
    if (pidChild==p4id){
        (*finishedPid)=(*finishedPid)+4;
    }
}
