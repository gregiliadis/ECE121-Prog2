/* HW4 - CE - 121 -
 * ---UTH--- 
 * DIMITRIS KEFALAS 2533
 * GRIGORIS ILIADIS 2522
--------------------------  
 * This program is a program execution enviroment that supports the following user commands:
	* exec <progname> <args>
	* term <pid>
	* sig  <pid> 
	* list
	* quit
 * With the command exec: Run the program 'progname' with arguments args, through separate
 process (fork-exec). 
 * With the command term: Send the SIGTERM signal to process with <pid>.
 * With the command sig : Send the SIGUSR1 signal to process with <pid>.
 * With the command list: Printing to the standar input, information about the ongoing processes:
 program ID, arguments, status (if running).
 * With the command quit: Terminate the execution enviroment.*/

/***************************************** HEADERFILE ************************************************/
#include "util.h"
/****************************************** FUNCTIONS ************************************************/
//A signal handler funtion: If the signal SIGCHLD or SIGUSR1 or SIGALRM is sent to our process then according to this function
//the global variable gotsig's value becomes enable. Because of this action our program will be informed that one of these signals
//has been received.
void sighandler ( int signum){

	switch (signum){
		case SIGCHLD: {got_signals[0] = ENABLED;break;}
		case SIGUSR1: {got_signals[1] = ENABLED;break;}
		case SIGALRM: {got_signals[2] = ENABLED;break;}
	}
}
//The main function, that implements the user commands of this program execution enviroment.
int main ( void ){
	char option[OPTION_SIZE], *temp, **executable;
	pid_t pid;
	ssize_t bytesReaden;
       	int cmd = 0, i;
	
	//Creating a sentinel node and initializing it. Informations about the process that 
	//are created from our execution enviromen and are active will be saved in this list.
	initializing_list();
	
	//Initializing signals by using sigaction. Calling the system call sigaction, in order to change the action taken 
	//by our process on receipt of SIGUSR1, SIGCLD and SIGALRM.
	initializing_signals ();
	
	while (1){
		//A 'for' loop, in order to initialize the option array with '\0' every time
		//the system call read is called, so no evidence of older commands remain in
		//this array.
		for (i=0; i < OPTION_SIZE; i++){
			option[i]= '\0';
		}
		//Since SA_RESTART flag is not used, every time that a signal is sent to our
		//process and we are blocked at this place the system call read will fail,
		//and the errno will be set to ErrorINTeRupted. If read has failed and the code 
		//doesn't exit, then we are sure that a signal has sent to our process.
		if ( (bytesReaden = read (STDIN_FILENO, option, OPTION_SIZE)) < 0){
			if (errno != EINTR){
				perror ("read");
				exit (EXIT_FAILURE);
			}
		}
		//A check whether read has failed or not.
		//If one byte is read this must be '\n', so no one event should take place, 
		//otherwise we should check which command has user input.
		if (bytesReaden > 1){
			//Making the last byte that is equal to '\n' equal to '\0'.
			option[bytesReaden-1] = '\0';
			//Creating a duplicate copy of option to temp, in order to be destroyed
			//from the strtok at the input_seperation, function.
			temp = strdup (option);
			//Input seperation is a function that creates a 2d char array, in which every collumn
			//of this array is a string equal to a part of option string between the space delimeter.
			//For example if the option is 'exec ./test1 -m 10 -b 0', the first collumn of the array will be
			//'./test', the second '-m', etc.
			input_separation (&executable, temp);
     			//If the user hasn't import a space then executable will be NULL, since strtok is used with space
			//delimeter.
			if (executable != NULL){ 
				//If the command is exec.
				if (strcmp ("exec", executable[0])==0){
					//The executing_program function: run the program 'progname' with arguments args, 
					//through separate process (fork-exec), adds to storage list information about this
					//process.   
					executing_program ( executable, option);
					//Variable cmd is actually a flag that is using every time that a valid command has	
					//input.
					cmd = 1;
				}
				//If the command is list.
				if (strcmp (option, "list") == 0){
					//Printing information about the processes in the storage list. 
					print_list ();
					cmd = 1;
				}
				//If the command is term.
				if (strcmp (executable[0], "term")==0){
					//A check whether user has input in the right way the command term.
					if (executable [1] != NULL){
						//Making the executable [1] which must be a number in ASCII to a pid_t.
						pid = atoi(executable[1]);
						//If this pid belongs to a process that is created from our execution enviroment
						//the signal SIGTERM will be sent to this process. 
						if ( find_node (pid) ){
							if (kill (pid, SIGTERM)){
								perror("kill");
								if (errno != ESRCH){	
									exit (EXIT_FAILURE);
								}
							}
						}
						else {
							printf ("shell: term: (%d) - No such process\n", pid);
						}	
					}
					else{
						printf ("term: usage: term [-n pid_num] pid | jobspec ...\n");
					}
					cmd = 1;
				}
				//If the command is sig.
				if (strcmp (executable[0], "sig")==0){  
					//A check whether user has input in the right way the command term
					if (executable [1] != NULL){
						//Making the executable [1] which must be a number in ASCII to a pid_t.
						pid = atoi(executable[1]);
						if (find_node (pid)){
						//If this pid belongs to a process that is created from our execution enviroment
						//the signal SIGUSR1 will be sent to this process. 
							if (kill (pid,SIGUSR1)){
								perror ("kill");
								if (errno != ESRCH){	
									exit (EXIT_FAILURE);
								}
							}
						}	
						else {
							printf ("shell: sig: (%d) - No such process\n", pid);
						}	
					}
					else{
						printf ("sig: usage: sig [-n pid_num] pid | jobspec ...\n");
					}
					cmd = 1;
				}
				//If the command is quit, list will be destroyed and also all the processes that our execution enviroment
				//manages.	
				if ( strcmp (executable[0], "quit") == 0 ) {
					clear_list ();
					//Freeing memory allocated.
					for (i=0; executable[i] != NULL; i++){
                                        	free (executable[i]);
					}
					free (executable);
					free (temp);
					break;
				}
				if (cmd == 0){
					printf ("%s: command not found\n",executable[0]);
				}
				cmd = 0;
				//Freeing memory allocated.
				for (i=0; executable[i] != NULL; i++){
                                        free (executable[i]);
				}
				free (executable);
			}
			free (temp);
		}
		//got_signals[0] is the volatile_sig_atomic variable which is ENABLED whenever a SIGCHLD is sent to our process.
		if (got_signals [0] == ENABLED){
			//Removing all the nodes whose process has been terminated.
			remove_node ();
			//Dissabling this variable.
			got_signals [0] = DISSABLED;		
		}
		//got_signals[1] is the volatile_sig_atomic variable which is ENABLED whenever a SIGCUSR1 is sent to our process.
		if (got_signals [1] == ENABLED){
			//Sending SIGUSR1 to all the process of our list.
			send_sigusr1 ();
			//Dissabling this variable.
			got_signals [1] = DISSABLED;
		}
		//got_signals[2] is the volatile_sig_atomic variable which is ENABLED whenever a SIGALRM is sent to our process.
		if (got_signals [2] == ENABLED){
			//Process_management is a function that checks which proccess from our list is running at this moment.
			//If there are processes in our shell, then the one that is running will be stoped and the next process
			//will continue its work and be running. 
			process_management ();
			//Dissabling this variable.
			got_signals [2] = DISSABLED;
			//Calling alarm.
			clock_initializing();
		}
		
	}

	return (0);
}


