/* HW3-CE-121-
 * ---UTH--- 
 * DIMITRIS KEFALAS 2533
 * GRIGORIS ILIADIS 2522
 * A program that is a part of a system reservation airline tickets which is running
 * local in one computer- more specifically this program has the agent role. The main function of 
 * this program is to send requests to the server for access in shared memory where information flights
 * and available positions are included, and senf information to the server about each reservation did. This
 * program has as argument the filepath to a stream socket for the necessary communication 
 * between server and agent.*/

/******************* HEADERFILE ***************************/
#include "util.h"
/******************* DEFINES ******************************/
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m" //white for black "\x1b[30m"
#define FIND "FIND"
#define RESERVE "RESERVE"
#define QUIT "EXIT"
/******************* PROTOTYPES **************************/
int searching_data ( unsigned long *head, int semid);
int reserve_tickets (unsigned long* head, int server_fd, int semid);
void menu (int socket_fd, unsigned long *head, int semid);
/******************* BODY OF FUNCTIONS *******************/
//In fuction main agent sends its pid to the server. If his request for connection
//is accepted then he is waiting to receive the semaphore id in order to start the 
//interaction with the user.
int main (int argc, char *argv[]){
    	int socket_fd, shmid;
    	struct sockaddr_un addr;
    	pid_t pid;
    	unsigned long *mem;
	int semid;

    	if (argc == 2){
		//Creating a stream socket file descriptor. This file descriptor will be used in order
		//to be connected with the server
        	socket_fd = mySocket (AF_UNIX, SOCK_STREAM, 0, __FILE__, __LINE__);
        	addr.sun_family = AF_UNIX;
        	strcpy ( addr.sun_path, argv[1]);
		//Connect our stream socket with the server's socket. 
        	myConnect ( socket_fd, (struct sockaddr*) &addr, sizeof (addr), __FILE__, __LINE__);
        	pid = getpid ();
		//Sending to server the process id. This action is equal to request for connection.
        	mySend ( socket_fd, (pid_t*) &pid, sizeof (pid_t), 0, __FILE__, __LINE__);
		//Receiving from the server our a value.
        	myRecv ( socket_fd, &shmid, sizeof (int), 0, __FILE__, __LINE__);
		//If value is equal to zero then the request is denied, other wise the value will 
		//be equal to shared memory id, so it is able to access the shared memory.
        	if (shmid == 0){
            		printf ("Try again later.\n");
            		exit (EXIT_FAILURE);
        	}
		//Receiving from the server the semaphore id.
        	myRecv ( socket_fd, &semid, sizeof (int), 0, __FILE__, __LINE__);
		//Matching mem with the shared memory.
        	mem = (unsigned long*) myShmat ( shmid, NULL, 0, __FILE__, __LINE__);
		//User interaction
        	menu ( socket_fd, mem, semid);
		close ( socket_fd );
		shmdt ( mem );
    	}

	return (0);
}
//A function searching fligths in the shared memory.
int searching_data ( unsigned long *head, int semid){
	char src[5], dest[5];
	int num, ret_val;
	char format[10];
	int flag = 0;
	struct flight_info *data;
	unsigned long flights, counter;
	struct sembuf op;
	
	//Getting the neccesary data for the flight to search.
	sprintf (format, " %%%ds",4);
	scanf (format, src);
	scanf (format, dest);
	scanf (" %d", &num);
        //Down the semaphore in order to be in the queue to acess the shared memory.
	op.sem_num = 0;
      	op.sem_op = DOWN;
 	op.sem_flg = 0;
	ret_val = mySemop (semid, &op, 1, __FILE__, __LINE__);
        //If mySemop returns -1 then the semaphore family has been removed.
	if ( ret_val == -1 ) {
        	return(-1);
	}
	//Initializng variable flights with the value of flights that is located at the first sizeof(unsigned long) bytes.
	flights = *head;
	//A loop to find out if there is any flight that matches with the user's input. With cyan color are printed all the flights that
	//are valid, with red all the flights that have less available seats than user asked. 
        for ( data =(struct flight_info*) (head + sizeof(unsigned long)), counter = 0; counter < flights;data=data+1, counter++) {
		if (strcmp (src, data->departure_airport) == 0 && strcmp (dest, data->arrival_airport) == 0){
			if (flag == 0){
				printf ("Flights found:\n");
				flag++;
			}
			if (num <= data->seats_available){
				printf ( ANSI_COLOR_CYAN "%s %s " ANSI_COLOR_RESET, data->airline, data->departure_airport);
				printf (ANSI_COLOR_CYAN "%s %d %d\n" ANSI_COLOR_RESET, data->arrival_airport, data->intermediate_stops, data->seats_available);
			}
			else{	
				if (data->seats_available < num){
					printf ( ANSI_COLOR_RED "%s %s " ANSI_COLOR_RESET, data->airline, data->departure_airport);
                                	printf ( ANSI_COLOR_RED "%s %d %d" ANSI_COLOR_RESET "\n", data->arrival_airport, data->intermediate_stops, data->seats_available);
				}
			}

		}

	}
	//If anyone flight was found then a message will be printed.
	if ( !flag ){
		printf ( ANSI_COLOR_RED "Flight from %s to %s is not provided from us.\n" ANSI_COLOR_RESET, src, dest);
	}
	
	return(0);
} 
//A function reserving the tickets. With green color is printed a message if agent can done the reservation.
//With red color an error message is printed if seats available is less than agent asked, or flight doesn't exist.
int reserve_tickets (unsigned long *head, int server_fd, int semid){
	char src[5], dest[5], airline[4];
	int num;
	char format[10];
	struct flight_info *data;
	unsigned long flights;
	unsigned long counter;
	struct sembuf op;
	
        //Getting the neccesary data for the flight to search.
	sprintf (format, " %%%ds",4);
        scanf (format, src);
        scanf (format, dest);
	scanf (" %3s",airline);
        scanf (" %d", &num);
	//Initializng variable flights with the value of flights that is located at the first sizeof(unsigned long) bytes.
	flights = *head;
	//Down the semaphore in order to be in the queue to access the shared memory.
	op.sem_num = 0;
	op.sem_op = DOWN;
  	op.sem_flg = 0;
       	//If mySemop returns -1 then the semaphore family has been removed.
	if ( mySemop (semid, &op, 1, __FILE__, __LINE__) == -1){
		return (-1);
	}
	
	
	//A loop to find out if there is any flight that matches with the user's input. With green color is printed the reservation,
        //with red an error message.
	for ( data = (struct flight_info*) (head+sizeof (unsigned long)), counter = 0 ; counter < flights ; data++, counter++ ) {
                if (strcmp (src, data->departure_airport) == 0 && strcmp (dest, data->arrival_airport) == 0 && strcmp (airline,data->airline) == 0){
			if (num <= data->seats_available){
				data->seats_available = data->seats_available - num;
				mySend (server_fd, src ,5, 0, __FILE__, __LINE__);
				mySend (server_fd, dest ,5, 0, __FILE__, __LINE__); 
				mySend (server_fd, data->airline, 4, 0, __FILE__, __LINE__);
				mySend (server_fd, &num, sizeof(int), 0, __FILE__, __LINE__);
				printf (ANSI_COLOR_GREEN "You reseved: %d tickets with %s airline for the journey from" ANSI_COLOR_RESET,num, data->airline);
				printf (ANSI_COLOR_GREEN " %s to %s." ANSI_COLOR_RESET "\n", src, dest);
				fflush (stdout);
				return(0);
			}
			else{	
				if (data->seats_available > 0){
					printf ( ANSI_COLOR_RED "From %s to %s flight only %d seats are available.\n" ANSI_COLOR_RESET, src, dest, data->seats_available);
					fflush (stdout);
					return(0);
				}
				else{
					printf (ANSI_COLOR_RED "All tickets from %s to %s are reserved.\n" ANSI_COLOR_RESET, src, dest);
					return(0);
				}
			}
		}
	}
	printf (ANSI_COLOR_RED "Flight from %s to %s is not provided from us.\n"ANSI_COLOR_RESET , src, dest);
	
	return(0);
}
//A function that prints the menu, and calls the corresponding function.
void menu (int socket_fd, unsigned long *head, int semid){
	char format[10];
	char selection[8];
	struct sembuf op;

	sprintf (format," %%%ds",7);
	do{
		printf ("----------------------------\n");
		printf ("FIND SRC DEST NUM\nRESERVE SRC DEST AIRLINE NUM\nEXIT\n");
		printf ("----------------------------\n");
		scanf (format, selection);
		//If user has insert the selection FIND.
		if (strcmp (selection, FIND) == 0){
			if ( searching_data( head, semid) == -1 ) {
				return;
			}
			//Up the semaphore in order to let the next on in the queue to access the shared memory.
			op.sem_num = 0;
                        op.sem_op = UP;
                        op.sem_flg = 0;
			mySemop (semid, &op, 1, __FILE__, __LINE__);
		}
		//If user has insert the selection RESERVE.
		if (strcmp (selection, RESERVE) == 0){
			if ( reserve_tickets ( head, socket_fd, semid) == -1){
				printf("Your reservation has not been registered.\n");	
                                return;
			}
			//Up the semaphore in order to let the next on in the queue to access the shared memory.	
			op.sem_num = 0;
                        op.sem_op = UP;
                        op.sem_flg = 0;
                        mySemop (semid, &op, 1, __FILE__, __LINE__);

		}
		//If user has insert the selection RESERVE.
		if (strcmp (selection, QUIT) == 0){
			return;
		}
		//If uses has insert a different selection.
		if (strcmp(selection, FIND) != 0 && strcmp (selection, RESERVE) != 0 && strcmp (selection, QUIT) != 0){
			printf ("Not a valid option.\n");
		}
	}while (1);
}
