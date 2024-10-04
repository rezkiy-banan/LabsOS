CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGETS = mycat mygrep

all: $(TARGETS)

mycat: mycat.c
	$(CC) $(CFLAGS) -o mycat mycat.c

mygrep: mygrep.c
	$(CC) $(CFLAGS) -D_GREP -o mygrep mygrep.c

clean:
	rm -f $(TARGETS) *.o