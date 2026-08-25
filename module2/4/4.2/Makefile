CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror

TARGET = priority_queue
TEST_TARGET = test_priority_queue
TEST_SRC = tests/test_main.c tests/test_priority_queue.c priority_queue.c

.PHONY: all test clean rebuild

all: $(TARGET)

$(TARGET): main.c menu.c menu.h priority_queue.c priority_queue.h
	$(CC) $(CFLAGS) main.c menu.c priority_queue.c -o $(TARGET)

$(TEST_TARGET): $(TEST_SRC) priority_queue.h tests/test_priority_queue.h
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_TARGET) -lcunit

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	$(RM) $(TARGET) $(TEST_TARGET)

rebuild: clean all
