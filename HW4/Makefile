CC = gcc
CFLAGS = -Wall -Wextra -g
OBJECTS = hw4 test

all: $(OBJECTS)
	echo Done
	
test: test.c
	$(CC) $(CFLAGS) test.c -o test

util.o: util.c util.h 
	$(CC) $(CFLAGS) -c util.c
	
hw4: util.o util.h hw4.c
	$(CC) $(CFLAGS)  util.o hw4.c -o hw4
	
clean:
	rm -f $(OBJECTS) util.o
