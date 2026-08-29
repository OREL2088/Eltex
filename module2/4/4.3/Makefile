CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic

TARGET = phonebook
TEST_TARGET = test_phonebook
SRC = main.c phonebook.c storage.c ui.c
TEST_SRC = tests/test_main.c tests/test_phonebook.c phonebook.c storage.c

all: $(TARGET)

$(TARGET): $(SRC) phonebook.h storage.h ui.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

$(TEST_TARGET): $(TEST_SRC) phonebook.h tests/test_phonebook.h
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_TARGET) -lcunit

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

rebuild: clean all

.PHONY: all test clean rebuild
