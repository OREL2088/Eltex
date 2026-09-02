CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2
LDLIBS = -pthread -lrt

TARGET = chat
OBJ = main.o common.o chat.o sender.o

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LDLIBS)

chat.o: chat.c chat.h
	$(CC) $(CFLAGS) -c chat.c

main.o: main.c chat.h
	$(CC) $(CFLAGS) -c main.c

common.o: common.c chat.h
	$(CC) $(CFLAGS) -c common.c

sender.o: sender.c chat.h
	$(CC) $(CFLAGS) -c sender.c

clean:
	rm -f $(OBJ) $(TARGET)

rebuild: clean $(TARGET)

.PHONY: clean rebuild
