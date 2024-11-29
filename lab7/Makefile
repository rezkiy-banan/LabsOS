CC = gcc
CFLAGS = -Wall -Wextra -std=c11

all: sender receiver

sender: sender.c
	$(CC) $(CFLAGS) -o sender sender.c -lrt -pthread

receiver: receiver.c
	$(CC) $(CFLAGS) -o receiver receiver.c -lrt -pthread

clean:
	rm -f sender receiver
