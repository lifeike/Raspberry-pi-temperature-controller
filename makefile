objects = $(patsubst %.c,%.o,$(wildcard *.c))
CC = gcc
CFLAGS = -g 
TARGET = a.out
$(TARGET):$(objects)
	$(CC) $(CFLAGS) -o $@ $^ -lsqlite3
clean:
	rm -rf *.o
