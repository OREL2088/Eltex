CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic

TARGET = phonebook
TEST_TARGET = test_phonebook

all: $(TARGET)

$(TARGET): main.c phonebook.c phonebook.h
	$(CC) $(CFLAGS) main.c phonebook.c -o $(TARGET)

TEST_SRC = tests/test_main.c tests/test_phonebook.c phonebook.c

$(TEST_TARGET): $(TEST_SRC) phonebook.h tests/test_phonebook.h
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_TARGET) -lcunit

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

rebuild: clean all

.PHONY: all test clean rebuild
