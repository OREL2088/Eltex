CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror

TARGET = permissions
TEST_TARGET = test_permissions

TEST_SRC = tests/test_main.c tests/test_permissions.c permissions.c

.PHONY: all test clean

all: $(TARGET)

$(TARGET): main.c permissions.c permissions.h
	$(CC) $(CFLAGS) main.c permissions.c -o $(TARGET)

$(TEST_TARGET): $(TEST_SRC) permissions.h tests/test_permissions.h
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_TARGET) -lcunit

test: $(TEST_TARGET) $(TARGET)
	./$(TEST_TARGET)

clean:
	$(RM) $(TARGET) $(TEST_TARGET) dummy_file
