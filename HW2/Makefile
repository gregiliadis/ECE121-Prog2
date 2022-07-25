CC = gcc
CFLAGS = -Wall -Wextra -g
OBJECTS = hw2 dirlist p2archive p2unarchive p2crypt

all: $(OBJECTS)
	echo Done
	
util.o: util.c util.h 
	$(CC) $(CFLAGS) -c util.c
	
dirlist: dirlist.c
	$(CC) $(CFLAGS)  util.o dirlist.c -o dirlist

p2archive: util.o util.h p2archive.c
	$(CC) $(CFLAGS)  util.o p2archive.c -o p2archive
	
p2crypt: util.o util.h p2crypt.c
	$(CC) $(CFLAGS) util.o p2crypt.c -o p2crypt
	
p2unarchive: util.o util.h p2unarchive.c
	$(CC) $(CFLAGS) util.o p2unarchive.c -o p2unarchive
	
hw2: util.o util.h hw2.c
	$(CC) $(CFLAGS) util.o  hw2.c -o hw2
clean:
	rm -f $(OBJECTS) util.o
