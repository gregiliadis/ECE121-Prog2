/* CE-121 2017-2018 UTH HW1
 * KEFALAS.2533-ILIADIS.2522
 * A program that administrates a database. The main fuctions of the program is 
 * importing, finding, executing and deleting files in the database.*/

/*******************LIBRARIES AND HEADERFILES********************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
/**************************DEFINES*******************************/
#define BLOCK 512
#define NAME_SIZE_FILEPATH 300
#define NAME_SIZE 160
/*************************PROTOTYPES*****************************/
ssize_t myWrite (int fileDescriptor, void* dataToWrite, int length);
ssize_t myRead (int fileDescriptor,void* dataToRead, int length);
int checkingFiles(int fd, char code[]);
off_t mylseek(int fd, off_t offset, int whence);
void codingDataBase (char *code);
int checkingFiles(int fd, char code[]);
void importing (char code[], int dataDescriptor);
void userInteraction (int fileDescriptor,int dataDescriptor2, char code[]);
void printingMenu();
off_t computingFileSize (int fileDescriptor);
char* seperatingName (char *name);
ssize_t contentsFile (int fileDescriptor, int dataDescriptor, off_t sizeFile);
void searchingData (int fd, char code[],char *nameFile, off_t *sizeFile, int *nameSize);
int find(char *nameInFile, char* userNameImport);
int typicalChecks (int fd,off_t sizeFile);
int looping (char code[], int fileDescriptor,int dataDescriptor,char input,char *userNameImport);
void export(int databaseDescriptor,char*filepath, off_t sizeFile);
void delete(int fileDescriptor,int dataDescriptor,off_t sizeFile,int nameSize,char *code, off_t sizeDataBase);
void fileNameInput (char code[],int dataDescriptor, int databaseDescriptor, char input);
/*************************FUNCTIONS******************************/
int main (int argc, char *argv[]){
    int dataDescriptor,dataDescriptor2;
    char codeDataBase[5];
    
    codingDataBase(codeDataBase);
    if (argc==2){
        dataDescriptor=open(argv[1],O_RDWR | O_CREAT,0666);
        if (dataDescriptor<0){
            printf ("%s\n",strerror(errno));
        }
        //dataDescriptor2 is a new fileDescriptor of the database that is used
        //in function delete
        dataDescriptor2=open(argv[1],O_RDWR,0666);
        if (dataDescriptor<0){
            printf ("%s\n",strerror(errno));
        }
        else {
            checkingFiles(dataDescriptor,codeDataBase);
            userInteraction(dataDescriptor,dataDescriptor2,codeDataBase);
        }
    }
    else{
        printf ("Not right arguments!\n");
        return (-1);
    }
    
    return (0);
}
/*Enriching the system call write, with the appropriate checks.*/  
ssize_t myWrite (int fileDescriptor, void* dataToWrite, int length){
    size_t writtenBytes=0;
    size_t bytesToWrite;
    
    bytesToWrite=length;
    //A loop in order to be sure that all bytes will be written
    do{
        if((int)(writtenBytes=(write(fileDescriptor, dataToWrite, bytesToWrite)))<0){
            printf ("%s\n",strerror(errno));
            
            exit(-2);
        }
        bytesToWrite=length-writtenBytes;
    }while(bytesToWrite!=0);
    
    return(writtenBytes);
}
/*Enriching the system call read, with the appropriate checks*/
ssize_t myRead (int fileDescriptor,void* dataToRead, int length){
    size_t readenBytes=0;
    int bytesToRead;
    
    bytesToRead=length;
    //A loop in order to be sure that all bytes will be readen
    do{
        if ((int)((readenBytes=read(fileDescriptor,dataToRead,bytesToRead)))<0){
            printf ("%s\n",strerror(errno));
        
            exit(-1);
        }
        if (readenBytes==0){
            break;
        }
        bytesToRead=length-readenBytes;
    }while((bytesToRead!=0));

    return (readenBytes);
}
/* Initializing an array with the magic number (DAtaBAse).This magic number will
 * be before the beginning of every file in the database, and also after of each 
 * content.*/
void codingDataBase (char *code){
    code[0] = 0xD;
    code[1] = 0xA;
    code[2] = 0xB;
    code[3] = 0xA;
    code[4] = '\0';
    
    return;
}
/* Doing the appropriate checks whether the file that user input belongs to a database 
 * or not.*/
int checkingFiles(int fd, char code[]){
    ssize_t readenBytes;
    char *buff;
    int codeLength;

    codeLength=strlen(code);
    buff=(char*) malloc(codeLength+1);
    if (buff==NULL){
        printf ("Cannot allocate memory!\n");
        exit(-1);
    }
    readenBytes=myRead(fd,(char*) buff,codeLength);
    buff[codeLength]='\0';
    
    if (readenBytes!=0 && strcmp(buff,code)!=0){
        printf ("Not a Data Base file.\n");
        exit(-1);
    }
    free(buff);
    if (readenBytes==0){
        myWrite(fd,code,strlen(code));
    }
    else{
        if(lseek(fd,0,SEEK_END)<0){
            printf ("%s\n",strerror(errno));
        }
    }
    
    return (1);
}
/*The actual enviroment where user interacts with the program.*/
void userInteraction (int fileDescriptor,int dataDescriptor, char code[]){
    char input;
    
    do{
        printingMenu();
        scanf (" %c", &input);
        switch (input){
            case 'i':
                    {
                        importing (code,fileDescriptor);break;
                    }
            case 'f':
                    {
                    }
            case 'e':
                    {    
                    }
            case 'd':
                    {
                        fileNameInput(code,fileDescriptor,dataDescriptor,input);break;
                    }
            case 'q':
                    {
                        if (close(fileDescriptor)){
                            printf ("%s\n",strerror(errno));
                            exit(-1);
                        }
                        if (close(dataDescriptor)){
                            printf ("%s\n",strerror(errno));
                            exit(-1);
                        }
                        return;
                    }
            default : 
                    {
                        printf ("Invalid option!\nTry again!\n");
                    }
        }
    }while (input != 'q');
    
    return;
}
/*The form of the Menu*/
void printingMenu(){
    
    printf ("i(mport) <name>:\nf(ind)   <name>:\ne(xport) <src> <dest>:\n");
    printf ("d(elete) <name>:\nq(uit):\n");
    printf ("-->");
    
    return ;
}
/*Importing file in the database.*/
void importing (char code[], int dataDescriptor){
    char name[NAME_SIZE_FILEPATH], *title;
    char format[10];
    int fileDescriptor,nameSize;
    off_t sizeFile;
    off_t databaseSize;
    
    databaseSize=computingFileSize(dataDescriptor);
    
    sprintf (format,"%%%ds",NAME_SIZE_FILEPATH-1);
    scanf (format,name);
    
    fileDescriptor=open(name,O_RDONLY);
    if (fileDescriptor<0){
        printf ("%s\n",strerror(errno));
        return;
    }
    title=seperatingName(name);
    //moving the cursor to the first pos of the Database
    mylseek(dataDescriptor,0,SEEK_SET);
    //A loop in orden to check whether the file is already in the database or not.
    if (databaseSize!=4){
        if (looping(code,dataDescriptor,0,'i',title)){
            return;
        }
    }
    //Moving the cursor at the end of the Database in order to import the new file
    mylseek(dataDescriptor,0,SEEK_END);
    nameSize=strlen(title);
    sizeFile=computingFileSize(fileDescriptor);
    //Because the system call write which is a part of the myWrite expands the size of
    //the file if it is at the last position of the file, a function like ftruncate in 
    //order to manually expand the size of the file is not needed.
    myWrite (dataDescriptor,(int*) &nameSize, sizeof(int));
    //Writing in the database the main fields of the file and its content.
    //In the database infos about the file such as its size,name,namelength is saved.
    myWrite(dataDescriptor,(char*)title,nameSize);
    myWrite(dataDescriptor,(off_t*)&sizeFile,sizeof(off_t));
    contentsFile(fileDescriptor,dataDescriptor,sizeFile);
    myWrite(dataDescriptor, (char*) code, strlen(code));
    
    if (fsync(dataDescriptor)){
        printf ("%s\n",strerror(errno));
        exit(-1);
    }
    
    if (close(fileDescriptor)){
        printf ("%s\n",strerror(errno));
        exit(-1);
    }

    return;
}
/* Because user may gives the whole filepath we have to crop and use only the 
 * name of the file.This function implements this task.*/
char* seperatingName (char *name){
    char *pos1, *pos2,*title;
    const char SLASH = '/';
    
    pos1=NULL;
    pos2=NULL;
    
    do{
        pos1=strchr(name,SLASH);
        if (pos1!=NULL){
            pos2=pos1;
            *pos2='\0';
            name=pos1+1;
        }
        
    }while (pos1!=NULL);
    
    if (pos1==NULL && pos2==NULL){
        title=strdup(name);
        if (title==NULL){
            printf ("Problem in allocating memory!\n");
            exit(-1);
        }
    }
    else{
        title=strdup(pos2+1);
        if (title==NULL){
            printf ("Problem in allocating memory!\n");
            exit(-1);
        }
    }
    
    return (title);
}
/*A function in which the size of a file is computed.*/
off_t computingFileSize (int fileDescriptor){
    off_t fileSize;
    
    if ((fileSize=lseek(fileDescriptor,0,SEEK_END))<0){
        printf ("%s\n",strerror(errno));
        exit(-1);
    }
    
    if ((lseek(fileDescriptor,0,SEEK_SET))<0){
        printf ("%s\n",strerror(errno));
        exit(-1);
    }
    
    return (fileSize);
}
/*Writing the contents of a file that fileDescriptor represents, to another file that dataDescriptor represents.*/
ssize_t contentsFile (int fileDescriptor, int dataDescriptor, off_t sizeFile){
    char *buff;
    int pos,how_many;
    ssize_t readBytes,writtenBits;
    
    pos=0;
    
    do{
        buff = (char*) malloc (BLOCK);
        if (buff==NULL){
            printf("Cannot allocate memory!\n");
            exit(-1);
        }
        if ((sizeFile-pos)>=BLOCK){
            how_many=BLOCK;
        }
        else{
            how_many=sizeFile-pos;
        }
        readBytes=myRead(fileDescriptor,buff,how_many);
        if (pos==0){
                pos=readBytes;
        }
        else{
                pos=pos+readBytes;
        }
        myWrite(dataDescriptor,buff,readBytes);
        
        free(buff);
    }while (pos<sizeFile);
    writtenBits= (ssize_t) pos;
    
    if (fsync(dataDescriptor)){
        printf ("%s\n",strerror(errno));
        exit(-1);
    }
 
    return (writtenBits);
}
/*Enriching the system call lseek, with the appropriate checks*/
off_t mylseek(int fd, off_t offset, int whence){
    off_t posAtFile;
    
    if ((posAtFile=lseek(fd,offset,whence))<0){
        printf("%s\n",strerror(errno));
        exit(-1);
    }
    
    return(posAtFile);
}
/*Decoding the main data about a file.In import a field of the file's name length,
 * the name,and the size of the of the file is set in the database,*/
void searchingData (int fd, char code[],char *nameFile, off_t *sizeFile, int *nameSize){ 
    char  *buff;
    
    buff=(char*) malloc(strlen(code));
    if (buff==NULL){
        printf ("Cannot allocate memory!\n");
        exit(-1);
    }
    myRead(fd,(char*)buff,strlen(code));
    free(buff);
    //Decoding the size of the name
    myRead(fd,(int*)nameSize,sizeof(int));

    //Transforming the file's name in string.
    myRead(fd,nameFile,nameSize[0]);
    nameFile[nameSize[0]]='\0';
    //Decoding the file's size
    myRead(fd,(off_t*) sizeFile, sizeof(off_t));
    
    return;
}
//Find is a function that detects whether the userNameImport is part of the
//nameInFile.If the user's input is '*' then all the names in the database will be printed.
int find(char *nameInFile, char* userNameImport){
    
    if ( (strstr(nameInFile,userNameImport)!=NULL) || strcmp(userNameImport,"*")==0){
        printf ("%s\n",nameInFile);
        return(0);
    }
    
    return(1);
}
    
//Checking whether the database is empty or not. If it is empty, 
//parts of the program where user wants to find,export or delete 
//a file stops immediately because there is nothing to do.
int typicalChecks (int fd,off_t sizeFile){
        
    if (sizeFile==4){
        printf ("Empty database. Please Import some data.\n");
        return(-1);
    }
    mylseek(fd,0,SEEK_SET);
    
    return (0);
}
//A function that makes the tough job of the program.Each function that carries out
//one of the main parts of the code (import,find,export,delete) calls looping in order
//to find the position of the file where the file that is searched is found. It actually 
//stops the cursor of the database just before the content about the file is searched.
//The second dataDescriptor that is passed as the third argument is used ONLY in the function
//delete.
int looping (char code[], int fileDescriptor,int dataDescriptor,char input,char *userNameImport){
    char filepath[NAME_SIZE_FILEPATH],format[10],nameInFile[NAME_SIZE];
    int nameSize;
    int filesFound;
    off_t databaseSize,byteChecked,sizeFile;
    
    if (input=='e'){
        sprintf (format,"%%%ds",NAME_SIZE_FILEPATH-1);
        scanf(format,filepath);
    }
    databaseSize=computingFileSize(fileDescriptor);
    //In every option except of the import, a check whether the database is
    //empty or not is done.
    if (input!='i'){
        if (typicalChecks(fileDescriptor,databaseSize)){
            return(-1);
        }
    }
    filesFound=1;    
    byteChecked=0;
    //moving the cursor to the first pos of the Database
    mylseek(fileDescriptor,0,SEEK_SET);
    
    //A loop in order to check whether the file is already in the database or not.
    do{
        searchingData(fileDescriptor,code,nameInFile,&sizeFile,&nameSize);
        if (input=='f'){
            filesFound=find(nameInFile,userNameImport)*filesFound;
        }
        else{
            if (!strcmp(userNameImport,nameInFile)){
                if (input=='i'){
                    printf ("File is already in the database.\n");
                    return(1);
                }
                if (input=='e'){
                    export(fileDescriptor,filepath,sizeFile);
                    return(1);
                }
                if (input=='d'){
                    delete(fileDescriptor,dataDescriptor,sizeFile,nameSize,code,databaseSize);
                    return(1);
                }
            }
        }
        //byteChecked is a a variable that counts how many bytes has been checked in every 
        //file that is checked is moved by:
        //sizeof(int)-->bytes used to save the nameSize
        //nameSize-->The actual size of the name that is been saved in the database.
        //strlen(code)-->The magic number or code 0x0d 0x0a 0x0b 0x0a that gives 
        //informations about the database.
        //sizeof(off_t)-->the number that is equal to the size of the file is a off_t variable.
        //sizeFile-->The size of the file.
        byteChecked=byteChecked+sizeof(int)+nameSize+strlen(code)+sizeFile+sizeof(off_t);
        mylseek(fileDescriptor,byteChecked,SEEK_SET);
    }while(byteChecked<databaseSize-(unsigned)strlen(code));
    
    if (filesFound==1 && input!='i'){
        printf ("File is not in the database.\n");
    }

    return(0);
}
//The first part of find,export and delete procedures, in which the name's file to searched in the
//database is setted.
void fileNameInput (char code[],int dataDescriptor, int databaseDescriptor, char input){
    char userNameImport[NAME_SIZE], format[10];
    
    sprintf (format,"%%%ds",NAME_SIZE-1);
    scanf (format,userNameImport);
    //looping is called in order to be sure whether the userNameImport belongs
    //to a filename that is in the database.
    looping (code, dataDescriptor,databaseDescriptor,input,userNameImport);
    
    return;
}
//The main part of the export procedure, where the file is exported.
void export(int databaseDescriptor,char*filepath, off_t sizeFile){
    int newFileDescriptor;
    //O_EXCL Ensure that this call creates the file: if this flag is specified 
    //in conjunction with O_CREAT, and pathname already exists, then open() will fail.
    //When these two flags are specified, symbolic links are not followed: 
    //if pathname is a symbolic link, then open() fails regardless of where 
    //the symbolic link points to. SOURCE:MAN PAGE 
    if ((newFileDescriptor=open(filepath,O_EXCL | O_WRONLY | O_CREAT,0666))<0){
        printf ("%s\n",strerror(errno));
        exit(-1);
    }
    //Calling the function in which the contents of the file are writing from the database
    //to a new file where the newFileDescriptor is setted.
    contentsFile (databaseDescriptor, newFileDescriptor,sizeFile);
    
    if (fsync(newFileDescriptor)){
        printf ("%s\n",strerror(errno));
        exit(-1);
    }
   
    if (close(newFileDescriptor)){
        printf ("%s\n",strerror(errno));
        exit(-1);
    }
    
    return;
}

//The main part of the delete procedure, where the file is deleted, and the file's 
//size is cropped.
void delete(int fileDescriptor,int dataDescriptor,off_t sizeFile,int nameSize,char *code, off_t sizeDataBase){
    off_t posFileToCopy, posFileToDEl;  
        
    //The position of the file that has to be deleted, is actually the position
    //that the next file has to be written in. In fact the only deleting that
    //takes place is the ftruncate. All the other operation that delete does
    //is to copy some datas from one position of the database (posFileToCopy) to
    //another (posFileToDEl).posFileToCopy is expressed from fileDescriptor's cursor
    //and posFileToDEl is expressed from dataDescriptor's cursor.
        
    //Checking whether the file to be deleted is the last in the database or not.
    posFileToCopy=mylseek(fileDescriptor,sizeFile,SEEK_CUR);
    if (posFileToCopy<sizeDataBase-(unsigned)strlen(code)){
        posFileToDEl=mylseek(fileDescriptor,-(off_t)strlen(code)-(off_t)sizeof(off_t)-(off_t)sizeof(int)-(off_t)nameSize-sizeFile,SEEK_CUR);
        mylseek(dataDescriptor,posFileToDEl,SEEK_SET);
        mylseek(fileDescriptor,posFileToCopy,SEEK_SET);
        contentsFile(fileDescriptor,dataDescriptor,sizeDataBase-posFileToCopy);
    }
        
    if (ftruncate(fileDescriptor,sizeDataBase-strlen(code)-sizeof(off_t)-sizeof(int)-nameSize-sizeFile)){
        printf ("%s\n",strerror(errno));
        exit(-1);
    }
    
    if (fsync(fileDescriptor)){
        printf ("%s\n",strerror(errno));
        exit(-1);
    }
        
    return;
} 

