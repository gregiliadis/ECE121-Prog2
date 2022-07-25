/* HW2-CE-121-
 * ---UTH--- 
 * DIMITRIS KEFALAS 2533
 * GRIGORIS ILIADIS 2522
 * Dirlist is a program that prints to its standar output the name of the regular files
 * in a folder. The filepath of the folder is passed as an argument.*/
/*********** LIBRARIES & HEADERFILES ***********/
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
/*********** FUNCTIONS ***********/
int main (int argc, char *argv[]){
    DIR* folder;
    struct dirent *nameInDir;
    
    if (argc==2){
        folder = opendir ( argv[1]);
        if ( folder == NULL){
            fprintf ( stderr, "%s.\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        for ( nameInDir = readdir(folder); nameInDir != NULL; nameInDir = readdir(folder)){
            if ( (nameInDir->d_type) == DT_REG){
                printf ("%s/", argv[1]);
                printf ("%s\n", nameInDir->d_name);
    		fflush(stdout); 
            }
        }
        if(closedir (folder)){
            fprintf (stderr,"%s @%d: %s.\n",__FILE__,__LINE__,strerror(errno));
            exit (EXIT_FAILURE);
        }
    }
    else{
        printf ("Invalid arguments!\n");
    }
    
    return (0);
}
