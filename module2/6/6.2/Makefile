CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Werror -pedantic
PICFLAGS := -fPIC
LDFLAGS := -Wl,-rpath,'$$ORIGIN'

TARGET := phonebook
LIBRARY := libphonebook.so
LIBRARY_OBJECTS := phonebook.o
APP_OBJECTS := main.o ui.o storage.o
TEST_TARGET := test_phonebook
TEST_OBJECTS := tests/test_main.o tests/test_phonebook.o storage.o

all: $(TARGET)

$(TARGET): $(APP_OBJECTS) $(LIBRARY)
	$(CC) $(CFLAGS) $(APP_OBJECTS) -L. -lphonebook $(LDFLAGS) -o $@

$(LIBRARY): $(LIBRARY_OBJECTS)
	$(CC) -shared -o $@ $^

main.o: main.c phonebook.h storage.h ui.h
	$(CC) $(CFLAGS) -c $< -o $@

phonebook.o: phonebook.c phonebook.h
	$(CC) $(CFLAGS) $(PICFLAGS) -c $< -o $@

ui.o: ui.c ui.h storage.h phonebook.h
	$(CC) $(CFLAGS) -c $< -o $@

storage.o: storage.c storage.h phonebook.h
	$(CC) $(CFLAGS) -c $< -o $@

tests/test_main.o: tests/test_main.c tests/test_phonebook.h phonebook.h
	$(CC) $(CFLAGS) -c $< -o $@

tests/test_phonebook.o: tests/test_phonebook.c tests/test_phonebook.h phonebook.h storage.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_TARGET): $(TEST_OBJECTS) $(LIBRARY)
	$(CC) $(CFLAGS) $(TEST_OBJECTS) -L. -lphonebook -lcunit $(LDFLAGS) -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET) $(APP_OBJECTS) $(LIBRARY_OBJECTS) \
		$(TEST_OBJECTS) $(LIBRARY)

rebuild: clean all

.PHONY: all test clean rebuild
