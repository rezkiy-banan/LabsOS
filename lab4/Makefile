CC = gcc
CFLAGS = -Wall -Wextra -std=c11

all: mychmod

mychmod: main.o
	$(CC) $(CFLAGS) -o mychmod main.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f *.o mychmod
