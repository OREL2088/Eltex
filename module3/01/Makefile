CC = gcc
CFLAGS = -Wall -Wextra

TARGET = copy
OBJ = main.o copy.o

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

main.o: main.c copy.h
	$(CC) $(CFLAGS) -c main.c

copy.o: copy.c copy.h
	$(CC) $(CFLAGS) -c copy.c

clean:
	rm -f $(OBJ) $(TARGET)
	rm -f *.copy
	rm -f file1.txt file2.txt empty.txt big.bin

rebuild: clean $(TARGET)

.PHONY: clean rebuild