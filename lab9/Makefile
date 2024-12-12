CC = gcc
CFLAGS = -Wall -Wextra -std=c11

SEMFILE = semfile

all: sender receiver $(SEMFILE)

$(SEMFILE):
	touch $(SEMFILE)

sender: sender.c
	$(CC) $(CFLAGS) -o sender sender.c -lrt -pthread

receiver: receiver.c
	$(CC) $(CFLAGS) -o receiver receiver.c -lrt -pthread

clean:
	rm -f sender receiver sender.lock shmfile $(SEMFILE)
