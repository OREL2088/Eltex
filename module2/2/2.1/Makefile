CC = gcc
CFLAGS = -Wall -Wextra -Werror

TARGET = phonebook
TEST_TARGET = test_phonebook

SRC = main.c phonebook.c
TEST_SRC = tests/test_main.c tests/test_phonebook.c phonebook.c


all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

$(TEST_TARGET): $(TEST_SRC)
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_TARGET) -lcunit

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

rebuild: clean all