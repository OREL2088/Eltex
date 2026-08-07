CC = gcc
CFLAGS = -Wall -Wextra -Werror

TARGET = phonebook

SRC = main.c phonebook.c

OBJ = $(SRC:.c=.o)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)