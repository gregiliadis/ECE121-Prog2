/* HW3-CE-121-
 * ---UTH--- 
 * DIMITRIS KEFALAS 2533
 * GRIGORIS ILIADIS 2522
 * A program that is a part of a system reservation airline tickets which is running
 * local in one computer- more specifically this program has the server role. The main function of 
 * this program is to accept requests from agents for access in shared memory where information flights
 * and available positions are included, and be informed about reservations that each agent did. This
 * program has as arguments the maximum number of agents that can serve, the name of a file that contents
 * information about the flights and also a filepath to a stream socket for the necessary communication 
 * between server and agent.*/
/******************* HEADERFILE *******************/
#include "util.h"
//The prototype of a struct that contents some information about each agent.
struct agent_info {
	int fd;
	pid_t pid;
	int ticket_sold;
};

/******************* PROTOTYPES OF FUNCTIONS *******************/
int free_agent (struct agent_info *head, int fd_agent, pid_t agent_pid, long max_agents);
struct agent_info *creat_agent_list ( long  max_agents); 
unsigned long *memory_set (FILE *fp, int *shmid);
unsigned long  flights_num (FILE *fp); 
void file_set ( unsigned long *mem, FILE *fp);
int remove_agent (struct agent_info *head, int fd_agent, long max_agents);
pid_t list_agents_pid (int fd_agent, struct agent_info *head, int *total_num, int sold, long max_agents);
ssize_t receiving_client_reservations (int client_fd, struct agent_info *head, long max_agents);
/******************* BODY OF FUNCTIONS *******************/
//In function main the handling of agents takes place.
int main (int argc, char *argv[]){
    	FILE *fp;
	char input;
    	pid_t agent_pid;
    	ssize_t read3;
    	int fd, sock_fd, fd_client, fd_max;
    	int shmid, value_to_send, ret_val;
    	struct sockaddr_un addr;
    	fd_set set, read_set;
    	unsigned long *shr_mem;
	struct agent_info *head;
    	int semid, ret;
	long max_agents, counter;
        struct timeval time_select;
	
    	if (argc == 4){
		//Initializing max_agents with the value which user gave as an argument.
		max_agents = atol ( argv[1]);
		if (max_agents <= 0){
			fprintf (stderr,"Not a valid number for agents./n");
			unlink(argv[3]);
			return (0);
		}
		//Opening the file that contains information about the flights.
        	fp = myFopen ( argv[2], "r+", __FILE__, __LINE__);
		//Creating the shared memory. The variable 'shmid' is initialized with the shared memory id.
		//shmid is neccesary to be send to the agent, in order agent to get access to the shared memory.
        	shr_mem = memory_set (fp, &shmid);
		//Creating the semapatophore family and initializing to 1. The variable 'semid' is initialized with the  semaphore id.
                //semid is neccesary to be send to the agent, in order agent to get access to the semaphore.
        	semid = mySemget ( IPC_PRIVATE, 1, S_IRWXU | IPC_CREAT, __FILE__, __LINE__);
        	mySemctl (semid, 0, SETVAL, 1, __FILE__, __LINE__);
		//Initializing fd_max, an important variable for agent handling.
		fd_max = 0;
		//Creating an array with max_agents data, every new agent that will be connected with the
		//server, some information about him for ex. agent's stream socket file descriptor, agent's pid and
		//the number of the total tickets have been sold will be saved in this array.
		head = creat_agent_list ( max_agents);
		//Creating a stream socket file descriptor. This file descriptor will be used for every new agent will
		//be connected with the server.
		sock_fd = mySocket (AF_UNIX, SOCK_STREAM, 0, __FILE__, __LINE__);
		if ( fd_max < sock_fd){
			fd_max = sock_fd;
		}
		addr.sun_family = AF_UNIX;
		strcpy (addr.sun_path, argv[3]);
		//Binding the stream socket with the name that user gave for a succesfull communication between server and agents. 
		myBind ( sock_fd, (struct sockaddr_un*) &addr,  sizeof (addr), __FILE__, __LINE__);
		myListen ( sock_fd, SOMAXCONN, __FILE__, __LINE__);
		//Initializing a set that will be used in select.
		FD_ZERO ( &set);
		//Adding to the set the socket's file descriptor.
		FD_SET ( sock_fd, &set);
		//Adding to the set the standar input. If the user inputs 'Q' or send the signal SIGINT 
		//the server will be terminated.
		FD_SET (STDIN_FILENO, &set);
        	//The handling of the agents begins.
		while (1){
			//Because server monitors some file descriptors that are ready to read, only the read set 
			//of select will be used.	
			read_set = set;
			//Monitor multiple file descriptors who are included in the read_set, waiting until one or more of them
			//become "ready" for reading.
			mySelect ( fd_max+1, &read_set, NULL, NULL, NULL, __FILE__, __LINE__);
                	//One or more file descriptor are ready for reading. A 'for' loop, in order to check which
			//file descriptor of the set are ready.
            		for (fd=0; fd <=fd_max; fd++){
				//If fd is ready for ready for reading.
				if ( FD_ISSET(fd, &read_set)){
					//If fd is equal to sock_fd, then a new agent has sent a request to connect with the server.
                    			if ( fd == sock_fd){
						//Creating a new file descriptor for the new agent.
                        			fd_client = myAccept ( sock_fd, NULL, 0, __FILE__, __LINE__);
						//Receiving agent's pid.
                    				myRecv (fd_client , &agent_pid, sizeof( pid_t ), 0,__FILE__, __LINE__);
						//Initializing the variable that server will send to the agent with the shared memory id.
                                		value_to_send = shmid;
						//If the return value of the function free_agent is 0, the maximum number of agents that 
						//user gave is connected.
                                		if ( free_agent( head, fd_client, agent_pid, max_agents) == 0 ) {
							//If there is no free space in the agent's array. 0 will be sent to the agent.
							//Agent will print an error message, because he cannot be connected with the server.
                                    			value_to_send = 0;
                                		}
						//Sending to the agent the value_to_send.
                                		mySend (fd_client, &value_to_send,sizeof(int), 0, __FILE__, __LINE__);
                                		if ( value_to_send == 0){
                                    			
                                    			//remove_agent ( head, fd_client, max_agents);
                                    			close (fd_client);
                                		}
						else{
							//Adding the new agent's file descriptor to the set.
                                                	FD_SET ( fd_client, &set);
                             				//Since new agent is able to be connected with the server,
							//if agent's fd is greater than fd_max, fd_max has to be equall with agent's file descriptor. 
							if ( fd_max < fd_client){
                                                        	fd_max = fd_client;
                                                	}

							printf ("Agent (%d) just connected.\n",agent_pid);
							fflush (stdout);
							mySend (fd_client, &semid, sizeof (int), 0, __FILE__, __LINE__);
						}
                        		}
                    			else{
						//If standar input file descriptor is ready to be read.
                        			if (fd == STDIN_FILENO){
                        				ret = scanf (" %c", &input);
							if ( input == 'Q' || ret == EOF){
								//Waiting to take access to the shared memory.	
								semaphore (semid, DOWN);
								//At this point we are sure that everyone before us has already finish, his reservations.
								//If an agent had priority and has serve a ticket, while server was blocked Downing the semaphore
								//his reservation will not be able to be seen from the server. So a new select solve this problem.
								//Removing from the set the sock_fd, A new agent should not be connected at these point.
								FD_CLR ( sock_fd, &set );
								if ( sock_fd == fd_max ){
									fd_max--;
								}
								//Removing fro, the set STDIN_FILENO, no one is able to interupt from the keyboard.
								FD_CLR ( STDIN_FILENO, &set );
								if ( STDIN_FILENO == fd_max ){
									fd_max--;
								}
								//Setting the select's timeout equal to zero. This actually gives a NON_BLOCK ability to select
								time_select.tv_sec = 0;
								time_select.tv_usec = 0;
								//Refreshing the read_Set.
								read_set = set;
								ret_val = mySelect ( fd_max + 1, &read_set, NULL, NULL, &time_select, __FILE__, __LINE__ );
								//Since the return value of select is the number of file
       								//descriptors contained in the three returned descriptor sets, a control whether was able to be read or not is necessary.
								if (ret_val != 0){
									for (fd = 0; fd <= fd_max ; fd++ ) {
										if ( FD_ISSET (fd, &read_set ) ){
											receiving_client_reservations (fd, head, max_agents);
										}
									}
								}  
                                                                //Destroying the semaphore family, so all agents will be informed that server is offline.
                    						mySemctl ( semid, 0, IPC_RMID, 0, __FILE__, __LINE__);
								//Refreshing file's information about the flights.
								file_set (shr_mem, fp);
								//Detouching the shared memory.
								if ( shmdt ( shr_mem)){
									perror ("shmdt");
									exit (EXIT_FAILURE);
								}
                            					//Printing the agent's pid that are online, and the total number of tickets, that each has reserved.
                						for ( counter = 0 ; counter < max_agents ; counter++ )	{
									if ( (head + counter ) -> fd != -1 ) {
										printf ("(%d): RESERVED %d TOTAL TICKETS.\n", ( head + counter ) -> pid, (head +counter) -> ticket_sold );
										close( (head + counter ) -> fd );
									}
								}
								//Freeing the memory allocated for the agent's array.		
								free (head);
								//Closing the socket fd.
								close ( sock_fd );
								//Removing from the namespace the socket.
								unlink (argv[3]);
								return (0);
							}
                        			}
						else{
							/*If anyone of the agents has reserved tickets, agent's action must be sent to the server.*/ 
                            				read3 = receiving_client_reservations (fd, head, max_agents); 
			     				/*If agent has sent 0 bytes then due to socket's properties. Server realizes that agent has been exited from the server.*/
							if ( read3 == 0){
								//Removing agent's fd from the set.
                               					FD_CLR (fd, &set);
								//Checking the value of fd_max and agent's fd.
                                				if (fd_max==fd){
                                    					fd_max--;
                                				}
								//Removing agent from the agent's array.
                                				remove_agent ( head, fd, max_agents );
								//Closing agent's file descriptor.
                                				close (fd);
                            				}
						}
                    			}
                		}
            		}
        	}
    	}
    return (0);
}
//File_set is a function in which the file that contains information about the flights is renewed.
void file_set ( unsigned long *mem, FILE *fp) {
    	int fd;
    	struct flight_info *data;
    	unsigned long flights;
	unsigned long counter;
	
	//Creating a file descriptor in order to truncate the file.
	//This file descriptor is created with fileno.
    	fd = fileno(fp);
    	if (fd == -1) {
        	perror("fileno");
        	exit(EXIT_FAILURE);
    	}
	//Truncating the file to 0.
    	if (ftruncate ( fd, 0) == -1){
        	perror("ftruncate");
        	exit(EXIT_FAILURE);
    	}
    	//Moving the cursor in the beginning.
	fseek( fp, 0,SEEK_SET);
    	//Give to variable flights, the proper value which is saved at the first sizeof(unsigned long) bytes of shared memory.
	flights = *mem;
	//A 'for' loop for writing to the file the new data.
    	for (counter = 0, data = (struct flight_info*) (mem + sizeof(unsigned long)) ; counter < flights; data++, counter++ ) {
		fprintf( fp, "%s %s %s %d %d\n" , data->airline, data->departure_airport, data->arrival_airport, data->intermediate_stops, data->seats_available);
		fflush (fp);
    	}
	//Be sure that new data is written in file.
	fsync (fd);
	//Closing the new file descriptor.
	close (fd);
	//Closing the file pointer.
	fclose (fp);
} 
//Iniatializing the memory with a number of flights and an array in which information about datas are saved in.
unsigned long *memory_set (FILE *fp, int *shmid) {
    	unsigned long int flights;
    	struct flight_info *data ;
    	unsigned long *mem_to_placed;
    	char format [10];
    	unsigned long int counter;
	
    	sprintf (format," %%%ds",4);
        //Counting the flights.
    	flights = flights_num (fp);
	//Creating the shared memory.
    	*shmid = myShmget(IPC_PRIVATE, (flights * sizeof (struct flight_info) + sizeof(unsigned long)), IPC_CREAT | S_IRWXU, __FILE__, __LINE__ );
    	mem_to_placed = (unsigned long*) myShmat ( *shmid, NULL, 0, __FILE__, __LINE__);
    	//Making the shared memory able to be deleted.
	myShmctl ( *shmid, IPC_RMID, NULL, __FILE__, __LINE__);
	//Initializing the first first sizeof(unsigned long) bytes with the value that is equal to the flights.
    	*mem_to_placed = flights;
	//A 'for' loop to write to the shared memory flights array with the proper information about each flight.
    	for (data = (struct flight_info*) (mem_to_placed + sizeof(unsigned long)), counter=0 ; counter < flights; counter++, data++){
		fscanf (fp," %3s",data->airline);
		fscanf (fp,format,data->departure_airport);
		fscanf (fp,format,data->arrival_airport);
		fscanf (fp," %d",&(data->intermediate_stops));
		fscanf (fp," %d",&(data->seats_available));
		
    	}	
    
	return(mem_to_placed);
}
//A function that counts the flights.
unsigned long int flights_num (FILE *fp) {
    	unsigned long flights;
    	char char_in_file;
    
	//Initializing char_in_file
	char_in_file = 'a';
	//This count is happen by counting the new line symbol '\n'.
    	for (flights = 0; char_in_file != EOF ;){
        	char_in_file = fgetc (fp);
        	if ( char_in_file == '\n' ){
            		flights ++;
        	}
    	}
    	fseek( fp, 0,SEEK_SET);
    	
	return(flights);
}
//Create agent's array, which will content information about each agent. 
struct agent_info *creat_agent_list ( long  max_agents ) {
    	struct agent_info *head;
    	long counter;
    	
    	head = ( struct agent_info *) malloc ( (size_t) (max_agents * sizeof (struct agent_info)));   
    	if ( head == NULL ){
        	fprintf(stderr, "Problem in allocaiting memory.\n");
        	exit (EXIT_FAILURE);
    	}
	//Initializing the agent's array.
    	for (counter = 0 ; counter < max_agents ; counter++) {
        	(head + counter) ->fd = -1;
        	(head + counter) ->pid = 0;
        	(head + counter) ->ticket_sold = 0;
   	 }
    
    	return (head);
}
//A neccesary control in order to be informed wheter is a free space for a new agent or not.
int free_agent (struct agent_info *head, int fd_agent, pid_t agent_pid, long max_agents){
    	long counter;
    
    	for (counter = 0; counter < max_agents ; counter++){
        	if ( ( head+counter) -> fd == -1){
            		(head + counter) -> fd = fd_agent;
            		(head + counter) -> pid = agent_pid;
            		return(1);
        	}
   	}
    
    	return (0);
}
//Removing an agent from the agent's array.
int remove_agent (struct agent_info *head, int fd_agent, long max_agents){
    	long counter;

    	for (counter = 0; counter < max_agents; counter++){
		if ( (head + counter) -> fd == fd_agent ) {
			printf ("Agent (%d) is offline.\n",(head+counter)->pid);
            		(head + counter)-> fd = -1;
            		(head + counter) -> pid = 0;
			(head + counter) -> ticket_sold = 0;
            		break;
        	}
    	}	
    
    	return (0);
}
//Finding from the array the pid of an agent according his file descriptor, and counting agent's total tickets sold.
pid_t list_agents_pid (int fd_agent, struct agent_info *head, int *total_num, int sold, long max_agents){
	long counter;

    	for (counter = 0; counter < max_agents; counter++){
        	if ( (head + counter) ->fd == fd_agent ) {
			( head + counter) -> ticket_sold = (head + counter) ->ticket_sold +  sold;
			*total_num = ( head + counter) -> ticket_sold;
            		return ((head+counter)->pid);
        	}
    	}

    	return (0);
}
//Receiving from agent informations about its reservations, or be informed if he is exited.
ssize_t receiving_client_reservations (int client_fd, struct agent_info *head, long max_agents){
	char src[5], dest[5], airline[4];
	int tickets_sold, total_tickets_sold;
	pid_t pid;
	ssize_t recv_bytes;

	recv_bytes = 0;
	recv_bytes = myRecv ( client_fd, src, 5, 0, __FILE__, __LINE__) + recv_bytes;
	recv_bytes = myRecv ( client_fd, dest, 5, 0, __FILE__, __LINE__) + recv_bytes;
	recv_bytes = myRecv ( client_fd, airline, 4, 0, __FILE__, __LINE__) + recv_bytes;
	recv_bytes = myRecv ( client_fd, &tickets_sold, sizeof(int), 0, __FILE__, __LINE__) + recv_bytes;
	pid = list_agents_pid (client_fd, head, &total_tickets_sold, tickets_sold, max_agents); 
	
	if ( recv_bytes != 0){
		printf ("(%d): RESERVED %d TICKETS IN FLIGHT FROM %s TO %s WITH %s. TOTAL: (%d)\n", pid, tickets_sold, dest, src, airline, total_tickets_sold); 
	}

	return (recv_bytes);
}
