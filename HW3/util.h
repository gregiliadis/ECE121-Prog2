#ifndef __UTIL_H__
#define __UTIL_H__

#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#define UP 1
#define DOWN -1

int mySocket ( int domain, int type, int protocol, char *file, int line);
int myBind ( int sock_fd, void* addr, socklen_t addrlen, char *file, int line);
int myListen ( int sock_fd, int backlog, char *file, int line);
int myAccept ( int sock_fd, void *addr, socklen_t *addrlen, char *file, int line);
int mySelect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout, char *file, int line);
ssize_t mySend(int sockfd, const void *buf, size_t len, int flags, char *file, int line);
ssize_t myRecv (int sock_fd, void *buf, size_t len, int flags, char *file, int line);
FILE *myFopen (char *filepath, const char *mode, char *file, int line );
int myShmget (key_t key, size_t size, int shmflag, char *file, int line );
void *myShmat ( int shmid, const void *shmaddr, int shmflag, char *file, int lines);
int myShmctl ( int shmid, int cmd, struct shmid_ds *data, char *file, int line);
int myConnect (int sock_fd, const struct sockaddr *addr, socklen_t addrlen, char *file, int line );
int mySemctl ( int semid, int semnum, int cmd, int parameter, char *file, int line );
int mySemop ( int semid, struct sembuf *sops, size_t nsops, char *file, int line);
int mySemget ( key_t key, int nsems, int semflag, char *file, int line );
void semaphore (int semid, int value);

struct flight_info {
    char airline[4];
    char departure_airport[5];
    char arrival_airport[5];
    int intermediate_stops;
    int seats_available;
};

#endif
