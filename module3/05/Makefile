CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
LDLIBS = -pthread -lrt

TARGETS = producer consumer
COMMON_OBJ = common.o

all: $(TARGETS)

producer: producer.o $(COMMON_OBJ)
	$(CC) $(CFLAGS) $^ $(LDLIBS) -o $@

consumer: consumer.o $(COMMON_OBJ)
	$(CC) $(CFLAGS) $^ $(LDLIBS) -o $@

%.o: %.c shared.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(TARGETS)

rebuild: clean all

.PHONY: all clean rebuild
