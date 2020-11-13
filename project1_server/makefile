CC = gcc
CFLAGS = -lsqlite3 -lpthread

all: server_fork server_thread

server_thread: crc.o socket_server_thread.c
	$(CC) $(CFLAGS) crc.o socket_server_thread.c -o server_thread 

server_fork: crc.o socket_server_fork.c
	$(CC) $(CFLAGS) crc.o socket_server_fork.c -o server_fork

crc.o: crc.c
	$(CC) -c crc.c

clean:
	rm *.o *.db server_fork server_thread
