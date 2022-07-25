#ifndef __UTIL_H__
#define __UTIL_H__
/******************* HEADERFILE & LIBRARIES **********************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
/************************ DEFINES ********************************/
#define RUNNING 1
#define BLOCKED 0 
#define OPTION_SIZE 256
#define ENABLED 1
#define DISSABLED 0
#define SUCCESS 0
#define TIMEOUT 20
/************************ PROTOTYPES ********************************/
typedef struct process_info Process_info;
struct process_info{
	pid_t pid;
	char *name;
	char *arguments;
	int run;
	struct process_info *next;
	struct process_info *prev;
};

/******************** GLOBAL VARIABLES ***************************/
Process_info *head;
Process_info *run;
volatile sig_atomic_t got_signals[3];
/*********************** PROTOTYPE OF FUNCTIONS *******************************/
void process_management ();
void clock_initializing ();
void signal_setting (int mode);
void initializing_list ();
void add_node  (pid_t pid, char *name, char *arguments);
void print_list ();
void remove_node ( );
int find_node (pid_t pid);
void clear_list ();
ssize_t myIntScanf(void *buff, size_t count);
void send_sigusr1 ();
int empty_list ();
void semaphore (int semid, int value);
void sighandler ( int signum);
void executing_program (char *executable[], char *input);
int initializing_signals ();
void input_separation ( char ***separate_array, char* option);

#endif
