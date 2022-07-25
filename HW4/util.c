#include "util.h"

//Initiliazing_list is a function that creates a doubly linked list with sentinel node.
void initializing_list (){
		
	head = (Process_info*) malloc (sizeof (Process_info));
	if (head == NULL){
		fprintf (stderr,"Memory cannot be allocated.\n");
		exit (EXIT_FAILURE);
	}
	head->pid = 0;
	head->name = NULL;
	head->arguments = NULL;
	head->run = BLOCKED;  
	head->next = head;
	head->prev = head;
}
//Adding a new node to the double linked sentinel.
void add_node ( pid_t pid, char *name, char *arguments ) {
	Process_info *curr;
	Process_info *new_node;

	for ( curr = head ; curr->next != head ; curr = curr->next );
	new_node = (Process_info *) malloc (sizeof (Process_info ));
	if ( new_node == NULL ) {    	
		fprintf (stderr,"Memory cannot be allocated.\n");
		exit (EXIT_FAILURE);
	}
	new_node->pid = pid;
	new_node->name = (char * )malloc (strlen(name) + 1);
	if ( new_node->name  == NULL ) {    	
		fprintf (stderr,"Memory cannot be allocated.\n");
		exit (EXIT_FAILURE);
	}
	strcpy ( new_node->name , name );
	new_node->arguments = (char *) malloc (strlen(arguments) + 1);
	if ( new_node->arguments == NULL ) {    	
		fprintf (stderr,"Memory cannot be allocated.\n");
		exit (EXIT_FAILURE);
	}
	strcpy ( new_node->arguments, arguments);
	
	new_node->run = BLOCKED;
	new_node->next = head;
	new_node->prev = curr;
	curr->next = new_node;
	head->prev = new_node;
}
//Removing from the doubly linked list with sentinel the node with the pid of the process
//that has terminated.
void remove_node ( ) { 
	Process_info *curr, *temp;

	for( curr = head->next ; curr != head ; curr = temp ) {
		temp = curr ->next;
		//By using the option WNOHANG, waitpids return immediately if the child has exited with return value sets to 
		//setted to pid, if the child hasn't terminated the the return value is equal to zero.
		if (waitpid (curr->pid, NULL, WNOHANG)){
			curr->prev->next = curr->next;
			curr->next->prev = curr->prev;
			//If this node is marked ass running, then run node will be next one.
			if (run == curr){
				run = run -> next;
				if ( empty_list() == 0 ) {	
					got_signals[2] = ENABLED;
				}
			}
			free ( curr );
		}
	}
}
//A funtion that according a pid, goes on the list and checks wheter exists a process with 
//this pid. If a process exists the return 1 otherwise 0 is returned.
int find_node (pid_t pid){
	Process_info *curr;
	
	for( curr = head->next ; curr != head ; curr = curr->next ) {
		if ( curr->pid == pid ) {
			return (1);
		}
	}
	
	return (0);
}
//A function that prints information about the process of our enviroment.
void print_list () {
	Process_info *curr;
	int retval;
	char argument [512];
	char format[10];
	size_t offset;
	
	//Since function of stdio.h and string.h is called, there are many posibilities to fail 
	//if one of these signals arrive. That's why the signals are blocked.
        signal_setting (SIG_BLOCK);
	sprintf (format," %%%ds", 511);
	for (curr = head->next ; curr != head ; curr = curr->next ){
		printf ("Pid: %d , name: (%s", curr->pid, curr->name);
		for (retval = 1, offset=0; retval != EOF;){
			retval = sscanf (curr->arguments + offset, format, argument);
			if (retval != EOF){
				printf (" ,%s",argument);
			}
			offset=strlen (argument)+1+offset;
		}
		printf (")");
		if ( curr->run == RUNNING){
			printf (" (R)");
		}
		printf ("\n");
	}
	if ( head->prev == head ) {
		printf ("No processes.\n");
	}
	signal_setting (SIG_UNBLOCK);
}
//A function that according the mode that is passed as an argument (SIG_BLOCK, SIG_UNBLOCK, SIG_SETMASK),
//changes the signal mask of the calling thread.
void signal_setting (int mode){
	sigset_t set;
	
	//Initializing the signal set.
	if (sigemptyset (&set)){
		perror ("Sigemptyset");
		exit (EXIT_FAILURE);
	
	}
	//Adding to the signal set the SIGUSR1
	if (sigaddset (&set, SIGUSR1) ){
		perror ("sigaddset");
		exit (EXIT_FAILURE);
	}
	//Adding to the signal set the SIGCLD
	if (sigaddset (&set, SIGCHLD) ){
		perror ("sigaddset");
		exit (EXIT_FAILURE);
	}
	//Adding to the signal set the SIGALRM
	if (sigaddset (&set, SIGALRM) ){
		perror ("sigaddset");
		exit (EXIT_FAILURE);
	}
	//Depending on the mode, do the apropriate action to the signals that are contained into the set.
	if (sigprocmask (mode, &set, NULL)){
		perror ("sigprocmask"); 
	}
}
//Clear list is a function that kills all the processes that are created from our executing enviroment, 
//and free the memory that is allocated for the list.
void clear_list () {
	Process_info *curr, *temp;
	
	//Blocking the signals. Since the signal SIGKILL will be sent to every child, SIGCHLD will received from
	//this process something that doesn't have any value. Also at this point our enviroment has to quit, so 
	//no update should be made.
	signal_setting (SIG_BLOCK);

	for( curr = head->next ; curr != head ; curr = temp ) {
		temp = curr->next;
		//Sending the SIGKILL signal to each process.
		if (kill (curr->pid, SIGKILL)) {
			//If the process has already terminated but because the list isn't updated,
			//our shell has the illusion that is still able to run, if a signal will be sent to this process
			//kill will fail with the errno setted to ESRCH.{
			if (errno != ESRCH){
				perror ("kill");
				exit (EXIT_FAILURE);
			}
		}
		free(curr);
	}
	free(head);
}
//Sending the signal sigusr1 to every process in our list.
void send_sigusr1 (){
	Process_info *curr;

	for( curr = head->next ; curr != head ; curr = curr->next ) {
		//If the process has already terminated but because the list isn't updated,
       	        //our shell has the illusion that is still able to run, if a signal will be sent to this process
                //kill will fail with the errno setted to ESRCH.{
		if (kill (curr->pid, SIGUSR1)){
			if (errno != ESRCH){
				perror ("kill");
				exit (EXIT_FAILURE);
			}
		}
	}
}
//A check whether the list is empty or not. If the list is empty one will be returned otherwise 0.
int empty_list (){
	
	if (head->next == head){
		return (1);
	}

	return (0);
}
//This function manages the timing of the processes.
void process_management (){
	//A Check whether list is empty.
	if (empty_list()){
		return;
	}
	//If run is equal with the head. Then the process that will be Stop / Cont has to be the next one.
	//This could happen due to one reason. If the last node while was running, was removed.
	if (run == head){
		run = head ->next;
	}
	//If the field run of the node is equal to RUNNING ( != BLOCKED) then this node has to stoped.
	//Although, in the case that the last node was removed, so run was equal to head @line 18, the run field of the node
	//is already blocked.
	if (run -> run != BLOCKED){
		run -> run = BLOCKED;
		if (kill (run->pid, SIGSTOP)){
			if (errno != ESRCH){
				perror ("Kill");
				exit (EXIT_FAILURE);
			}
		}
		//Making run equal to the next node, in order to send to the stoped next process SIGCONT.
		if ( run -> next != head ) {
			run = run -> next;
		}
		else{
		//If the running node is the last node before the sentinel.
		//run shouldn't become equal to head, but equal to the next node after head.
			run = head -> next;
		}
	}
	//Sending signal SIGCONT to the process that has to continue.
	if (kill (run->pid, SIGCONT)) {
		if (errno != ESRCH){
			perror ("Kill");
			exit (EXIT_FAILURE);
		}
	}
	
	run -> run = RUNNING ;
}
//The executing_program function: run the program 'progname' with arguments args, 
//through separate process (fork-exec), adds to storage list information about this
//process.
void executing_program( char *executable[], char *input){
	pid_t pid;
	struct sigaction act_default;

	//Since the child process inherits the signal handling of the parent, and the block mask
	//this struct sigaction act_default, is used in order the restore action about this signals
	//in the child process.
	act_default.sa_handler = SIG_DFL;
	//Initializing the mask of the struct sigaction.
	//This action actually leaves the sa_mask empty.
	sigemptyset ( &act_default.sa_mask);
	//Creating the child proccess.
	if ((pid = fork()) == 0){
		//Restoring the SIGCHLD to its default action.
		if ( sigaction ( SIGCHLD, &act_default, NULL ) == -1 ) {
                	perror ("sigaction");
                	exit (EXIT_FAILURE);
        	}
		//Restoring the SIGDUSR1 to its default action.
		if ( sigaction ( SIGUSR1, &act_default, NULL ) == -1 ) {
                	perror ("sigaction");
                	exit (EXIT_FAILURE);
        	}
		//Restoring the SIGALRM to its default action.
		if ( sigaction ( SIGALRM, &act_default, NULL ) == -1 ) {
                	perror ("sigaction");
                	exit (EXIT_FAILURE);
        	}
		//Calling execvp.
		if (execvp (executable[1], executable + 1) == -1){
			if (errno == ENOENT){
				fprintf (stderr,"shell: %s: No such file or directory\n",executable[1]);
			}
			else{
				perror("execvp");
			}
			exit (EXIT_FAILURE);
		}
	}
	else{
		//Send the SIGSTOP signal to the child process.
		if ( kill (pid, SIGSTOP)){
			if ( errno != ESRCH){
				perror ("kill");
				exit (EXIT_FAILURE);
			}
			
		}
		//If the list is empty, then the child should continue immediately.
		if (empty_list()){
			//Create a new node to the list and add informations about the process.
			add_node (pid, executable[1], input + strlen(executable[0]) + strlen(executable[1]) + 2 );
			//The run global variable is set equal to this new node.
			run = head ->next;
			run -> run = RUNNING;
			//Activating the alarm got signal variable, so the alarm will start again.
			got_signals[2] = ENABLED;
			//Send the SIGCONT signal to the child process.		
			if (kill (pid, SIGCONT)){
				perror("kill");
				exit (EXIT_FAILURE);
			}
		}
                else{
			//Create a new node to the list and add informations about the process.
			add_node (pid, executable[1], input + strlen(executable[0]) + strlen(executable[1]) + 2 );
		}
	}
}
//This function determines the actions of the programm 
//when one of the SIGCHLD, SIGUSR1 and SIGALRM signals has been recieved.
int initializing_signals (){
	struct sigaction act_child;
	struct sigaction act_alarm;
	struct sigaction act_usr1;
	
	//Initialize signal mask.
	memset (&act_child, 0, sizeof (act_child));
	//Define the signal handling function.
	act_child.sa_handler = sighandler;
	//Flag SA_NOCLDSTOP is used in order to not  receive notification when child processes stop or resume.
	//Flag SA_NOCLDWAIT is used in order to not transform children processes into zombies when they terminate.
	act_child.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT;
	//The third argument of sigaction is NULL because there is no need
	// to keep the old action when a signal is send.
	if ( sigaction ( SIGCHLD , &act_child, NULL) == -1){
		perror ("sigaction");
		exit (EXIT_FAILURE);
	}
	//Initialize signal mask.
	memset (&act_usr1, 0, sizeof (act_usr1));
	//Define the signal handling function.
	act_usr1.sa_handler = sighandler;
	//The third argument of sigaction is NULL because there is no need 
	//to keep the old action when a signal is send.
	if ( sigaction ( SIGUSR1, &act_usr1, NULL) == -1){
		perror ("sigaction");
		exit (EXIT_FAILURE);
	}
	//Initialize signal mask.
	memset (&act_alarm, 0, sizeof (act_alarm));
	//Define the signal handling function.
	act_alarm.sa_handler = sighandler;
	//The third argument of sigaction is NULL because there is no need 
	//to keep the old action when a signal is send.
	if ( sigaction ( SIGALRM, &act_alarm, NULL ) == -1 ) {
		perror ("sigaction");
		exit (EXIT_FAILURE);
	}

	return (0);

}
//This function sets an alarm.  
void clock_initializing () {
	//Alarm command when is called sends a SIGALRM signal after TIMEOUT seconds. 
	alarm ( TIMEOUT );
}
//Function input_separation separates from a string input all individual strings that are separated with space character.
//Also saves the separated strings into a 2d array to whom points a triple character pointer. 
//Triple pointer is used in order to be able to extract the array out of the function.  
void input_separation ( char ***separate_array, char *input ) {
	int spaces,counter;
	char *token;		
	
	//Find out the number of strings to be separated by counting the intermediate spaces.
	for (spaces=1, counter=0; input[counter] != '\0'; counter++){
		if (input[counter] == ' '){
			spaces++;
		}
	}
	//Allocate memory for the first dimension of the array.
	*separate_array = (char **)malloc( (spaces + 1 ) * sizeof(char*));
	if ( *separate_array == NULL ) {
		fprintf (stderr,"Memory cannot be allocated.\n");
		exit(EXIT_FAILURE);
	}
	//Function strtok finds the first delimeter (second argument) in the input string,
	//replaces the delimeter with \0 and returns a pointer to the start of the next token string. 
	token = strtok ( input , " " ); 
	counter = 0;
	//If there are no tokens str return NULL and the function returns.
	if (token == NULL){
		separate_array = NULL;
		return;
	}
	counter = 0;
	//Allocate memory for the first row of the array.
	separate_array[0][counter] = (char*) malloc (strlen (token) +1); 
	if (*separate_array[counter]  == NULL){
		fprintf (stderr,"Memory cannot be allocated.\n");
		exit (EXIT_FAILURE);
	}
	//Insert the string into the array.
        strcpy (separate_array[0][counter], token);
	//Only in first call of strtok argument 1 is input string, in every other call is NULL. 
	for (counter = 1 ; (token = strtok( NULL, " " )) != NULL ; counter++ ){
		//Allocate memory for one row at a time.
		separate_array[0][counter] = (char*) malloc (strlen (token)+1);
		if (separate_array[0][counter]  == NULL){
			fprintf (stderr,"Memory cannot be allocated.\n");
			exit (EXIT_FAILURE);
		}
		//Insert the string into the array.
		strcpy (separate_array[0][counter], token);
		
	}
	//Last row of the array is empty (pointer set to NULL) in order to be able to find the end 
	//of the array without knowing the number of rows.
	separate_array[0][counter] = NULL;
	
	return;
}
