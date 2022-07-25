#include "util.h"

int mySocket ( int domain, int type, int protocol, char *file, int line){
    int sock_fd;
    
    sock_fd = socket( domain, type, protocol);
    if ( sock_fd == -1){
            fprintf ( stderr, "%s @ %d : %s.\n", file, line, strerror( errno));
            exit (EXIT_FAILURE);
    }
    
    return (sock_fd);
}

int myBind ( int sock_fd, void* addr, socklen_t addrlen, char *file, int line){
    
    if ( bind (sock_fd,(struct sockaddr*) addr, addrlen) == -1){
        fprintf ( stderr, "%s @ %d : %s.\n", file, line, strerror(errno));
        exit (EXIT_FAILURE);
    }
    
    return (0);
}

int myListen ( int sock_fd, int backlog, char *file, int line) {
    if ( listen ( sock_fd, backlog) == -1){
        fprintf ( stderr, "%s @ %d : %s.\n", file, line, strerror(errno));
        exit (EXIT_FAILURE); 
    } 
    
    return(0);
}

int myAccept ( int sock_fd, void *addr, socklen_t *addrlen, char *file, int line){
    int fd_client;
    
    fd_client = accept ( sock_fd, (struct sockaddr *)addr, addrlen);
    if ( fd_client == -1 ){
        fprintf ( stderr, "%s @ %d : %s.\n", file, line, strerror(errno));
        exit (EXIT_FAILURE);
    }
    
    return ( fd_client );
}

int myConnect (int sock_fd, const struct sockaddr *addr, socklen_t addrlen, char *file, int line ) {
    
    if ( connect( sock_fd, addr, addrlen) == -1) {
        fprintf ( stderr, "%s @ %d : %s.\n", file, line, strerror(errno));
        exit (EXIT_FAILURE);
    }
return(0);
}

ssize_t myRecv (int sock_fd, void *buf, size_t len, int flags, char *file, int line) {
    ssize_t bytesReceived;
    size_t counter;
    
    counter = 0;
    do{
        bytesReceived = recv ( sock_fd, (void *)buf + counter, len, flags);
        if ( bytesReceived == -1){
            fprintf ( stderr, "%s @ %d : %s.\n", file, line, strerror(errno));
            exit (EXIT_FAILURE);
        }
        else {
            if ( bytesReceived <= (ssize_t) len ){
                counter = counter + (size_t) bytesReceived;
            }
            len = len - counter; 
        }
    }while ( len > 0 && bytesReceived != 0);
    
    return ( counter);
}

ssize_t mySend(int sock_fd, const void *buf, size_t len, int flags, char *file, int line){
    ssize_t bytesSent;
    size_t counter;
    
    counter=0;
    do{
        bytesSent = send ( sock_fd, (void *)(buf + (int)counter), len, flags);
        if ( bytesSent == -1){
            fprintf ( stderr, "%s @ %d : %s.\n", file, line, strerror(errno));
            exit (EXIT_FAILURE);
        }
        else {
            if ( bytesSent <= (ssize_t) len ){
                counter = counter + (size_t) bytesSent;
            }
            len = len - counter;
        }
    }while ( len > 0 );
    
    return ( counter );
}

int mySelect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout, char *file, int line){
    int retval;
    
    retval = select (nfds, readfds, writefds, exceptfds, timeout);
    if ( retval == -1){
            fprintf ( stderr, "%s @ %d : %s.\n", file, line, strerror(errno));
        exit (EXIT_FAILURE);
    }
    
    return (retval);
}

int myShmget (key_t key, size_t size, int shmflag, char *file, int line ) {
    int shmid;
    
    if ( (shmid = shmget( key, size, shmflag)) == -1) {
        fprintf (stderr,"%s @ %d: ", file, line);
        perror("shmget");
        exit(EXIT_FAILURE);
    }
return(shmid);
}

FILE *myFopen (char *filepath, const char *mode, char *file, int line ) {
        FILE *fp;
        
        fp = fopen ( filepath, mode);
        if ( fp ==NULL ){
            fprintf (stderr,"%s @ %d: ", file, line);
            perror("fopen");
            exit(EXIT_FAILURE);
        }
 return(fp);   
}

void *myShmat ( int shmid, const void *shmaddr, int shmflg, char *file, int line) {
    void *p;
    
    p = (void *) shmat (shmid, shmaddr, shmflg );
    if ( p == (void *)-1 ) {
        fprintf (stderr,"%s @ %d: ", file, line);
        perror("shmat");
        exit(EXIT_FAILURE);
    }
return(p);
}

int myShmctl ( int shmid, int cmd, struct shmid_ds *data, char *file, int line) {
    int ret_val;
    
    ret_val = shmctl (shmid, cmd, data );
    if ( ret_val == -1 ){
        fprintf (stderr,"%s @ %d: ", file, line);
        perror("shmctl");
        exit(EXIT_FAILURE);
    }
return(ret_val);
}

int mySemop ( int semid, struct sembuf *sops, size_t nsops, char *file, int line) {
    
    if ( semop( semid, sops, nsops ) == -1 ) {
        if (errno == EIDRM || errno == EINVAL) {
            fprintf (stderr, "Server is offline.\n");
            return (-1);
        }
        else {
            fprintf (stderr,"%s @ %d: ", file, line);
	        perror("semop");
        	exit(EXIT_FAILURE);
 	}
   }
	return (0);
}

int mySemctl ( int semid, int semnum, int cmd, int parameter, char *file, int line ) {
    int ret_val;
    
    ret_val = semctl ( semid, semnum, cmd, parameter );
    if ( ret_val == -1 ) {
        fprintf (stderr,"%s @ %d: ", file, line);
        perror("semctl");
        exit(EXIT_FAILURE);
    }
return(ret_val);
}

int mySemget ( key_t key, int nsems, int semflag, char *file, int line ) {
    int semid;
    
    semid = semget ( key, nsems, semflag );
    if ( semid == -1 ){
        fprintf (stderr,"%s @ %d: ", file, line);
        perror("semget");
        exit(EXIT_FAILURE);
    }
return(semid);
}

void semaphore (int semid, int value){
    struct sembuf op;
    
    op.sem_num = 0;
	op.sem_op = value;
  	op.sem_flg = 0;
    /*printf ("Value is %d", value);*/
    mySemop (semid, &op, 1, __FILE__, __LINE__);
    
}
    
        
