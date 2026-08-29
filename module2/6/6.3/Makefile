CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Werror
PICFLAGS := -fPIC
LDLIBS := -ldl

TARGET := calculator
TARGET_OBJECTS := main.o ui.o calculator.o
TEST_TARGET := test_calculator
TEST_OBJECTS := tests/test_main.o tests/test_calculator.o calculator.o
PLUGIN_NAMES := addition subtraction multiplication division
PLUGIN_LIBRARIES := $(addprefix plugins/lib,$(addsuffix .so,$(PLUGIN_NAMES)))

.PHONY: all test clean rebuild

all: $(TARGET) $(PLUGIN_LIBRARIES)

$(TARGET): $(TARGET_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

main.o: main.c calculator.h ui.h
	$(CC) $(CFLAGS) -c $< -o $@

ui.o: ui.c ui.h calculator.h
	$(CC) $(CFLAGS) -c $< -o $@

calculator.o: calculator.c calculator.h plugin.h
	$(CC) $(CFLAGS) -c $< -o $@

tests/test_main.o: tests/test_main.c tests/test_calculator.h
	$(CC) $(CFLAGS) -c $< -o $@

tests/test_calculator.o: tests/test_calculator.c tests/test_calculator.h calculator.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS) -lcunit

plugins/lib%.so: plugins/%.c plugin.h
	$(CC) $(CFLAGS) $(PICFLAGS) -shared $< -o $@

test: all $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	$(RM) $(TARGET) $(TEST_TARGET) $(TARGET_OBJECTS) $(TEST_OBJECTS) \
		$(PLUGIN_LIBRARIES)

rebuild: clean all
