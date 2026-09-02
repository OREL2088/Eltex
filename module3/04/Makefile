CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -O2

TARGETS = producer consumer
COMMON_OBJ = common.o

all: $(TARGETS)

producer: producer.o $(COMMON_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

consumer: consumer.o $(COMMON_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c shared.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(TARGETS)

rebuild: clean all

.PHONY: all clean rebuild
