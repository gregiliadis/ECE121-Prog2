CC = gcc
CFLAGS = -Wall -Wextra -g
OBJECTS = server agent

all: $(OBJECTS)
	echo Done
	
util.o: util.c util.h 
	$(CC) $(CFLAGS) -c util.c
	
server: util.o util.h server.c
	$(CC) $(CFLAGS)  util.o server.c -o server
	
agent: util.o util.h agent.c
	$(CC) $(CFLAGS) util.o agent.c -o agent
	
clean:
	rm -f $(OBJECTS) util.o
