CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -O2

TARGETS = server client

all: $(TARGETS)

server: server.o protocol.o
	$(CC) $(CFLAGS) $^ -o $@

client: client.o protocol.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c protocol.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(TARGETS)

rebuild: clean all

.PHONY: all clean rebuild
