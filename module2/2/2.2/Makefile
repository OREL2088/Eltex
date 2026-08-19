CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror

TARGET = calculator
TEST_TARGET = test_calculator

TEST_SRC = tests/test_main.c tests/test_calculator.c calculator.c

.PHONY: all test clean rebuild

all: $(TARGET)

$(TARGET): main.c calculator.c calculator.h
	$(CC) $(CFLAGS) main.c calculator.c -o $(TARGET)

$(TEST_TARGET): $(TEST_SRC) tests/test_calculator.h calculator.h
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_TARGET) -lcunit

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	$(RM) $(TARGET) $(TEST_TARGET)

rebuild: clean all
