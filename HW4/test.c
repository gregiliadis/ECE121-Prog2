/* HW4-CE-121-
 * ---UTH--- 
 * DIMITRIS KEFALAS 2533
 * GRIGORIS ILIADIS 2522

 * This program takes arguments such as: -m M -b B, where M is the maximum number of repeats and B
 * a value tath determines whether SUGSUR1 will be blocked or not. A counter that is initialized to 0
 * and every five seconds prints its pid the value of counter and the maximum number of repeats. When
 * counter is equal to the maximum number, program terminates. If as an argument is passed the value
 * (-b 1), then the program blocks the SIGUSR1 signal for the first half repeat, and then it unblocks
 * it. Otherwise, the SIGUSR1 signal is unblocked the whole time. If a SIGUSR1 signal is received the 
 * is restored to 0.*/

/******************* HEADERFILE & LIBRARIES **********************/
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
/************************ DEFINES ********************************/
#define BLOCKED 1
/******************** GLOBAL VARIABLES ***************************/
volatile sig_atomic_t gotsig = BLOCKED;
/*********************** FUNCTIONS *******************************/
//A signal handler funtion: If the signal SIGUSR1 is sent to our process then according to this function
//the global variable gotsig's value becomes equal to 0. Because of this action our program will be informed 
//that a SIGUSR1 is received.
void signal_handler (int signum){
	if (signum == SIGUSR1){
		gotsig = 0;
	}
}
//Main Function: The main function of our program.
int main (int argc, char *argv[]){
	int counter, times, value, flag;
	struct sigaction act;
	pid_t pid;
	sigset_t set;
	
	//A check whether user has insert the right arguments or not.
	if (argc == 5){
		if (strcmp ("-m",argv[1]) == 0){
			if (strcmp ("-b", argv[3]) == 0){
				//Converting the maximum number of repeats from ASCII into int.
				times = atoi (argv[2]);
				//Converting the value that is passed as fourth parameter from ASCII into int.
				value = atoi (argv[4]);
				//If the user hasn't insert right values for these two variables, an error message
				//is printed.
				if ( !((value == 1 || value == 0) && (times > 0)) ){
					printf ("Invalid arguments, try again!\n");
					exit (EXIT_FAILURE);
				}
				//If the user wants to BLOCK the signal SIGUSR1 for the first half repeats.
				if (value == BLOCKED){
					//Initializing the signal set.
					if (sigemptyset (&set)){
						perror ("Sigemptyset");
						exit (EXIT_FAILURE);
					}
					//Adding to the signal set the SIGUSR1
					if (sigaddset (&set, SIGUSR1) ){
						perror ("sigaddset");
					}
					//BLOCKING the signals that is contained into the set.
					if (sigprocmask (SIG_BLOCK, &set, NULL)){
						perror ("sigprocmask"); 
					}
				}
				//Initializing important variables that will be used as parameters in the sigaction.
				memset (&act, 0, sizeof (act));
				act.sa_handler = signal_handler;
				//Calling the system call sigaction, in order to change the action taken by our process 
				//on receipt of SIGUSR1. 
				if ( sigaction ( SIGUSR1, &act, NULL) == -1){
					perror ("Sigaction");
					exit (EXIT_FAILURE);
				}
				//A 'for' loop in which a counter is initialized to 0, and every five seconds prints 
				//its pid, the value of counter and the maximum number of repeats are printed, until
				//the counter's value will be equal to the maximum number of repeats. 
				for ( counter = 0, pid = getpid(), flag = 0; counter <= times; counter++){
					//A check in order to be sure whether our process has received a SIGUSR1 signal or not.
					//Since the global volatile_sigatomic variable is initialized to one if gotsig's value is 
					//equal to zero, then a signal SIGUSR1 has been received.
					if (gotsig == 0){
						counter = 0;
						gotsig++;
					}	
					printf ("(%d): %d / %d\n", pid, counter, times);
					sleep (5);
					//A variable flag is used in order to be sure that if the SIGUSR1 is blocked,
					//only one time will be unblocked indepenedantly how many times SIGUSR1 is received.
					if ( (value == BLOCKED) && (counter == (times / 2)) && flag == 0){
						if (sigprocmask (SIG_UNBLOCK, &set, NULL)){
                                        		perror ("sigprocmask");
							exit (EXIT_FAILURE);
						}
						flag++;
					}
				}
			}
		}		
	}
	else{
		printf ("Invalid arguments.\n");
	}
	
	return (0);
}
